from picamera import PiCamera
from picamera.array import PiRGBArray
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import time
import numpy as np
import brickpi3
import cv2
import math
import zbar

# global vars
marker_offset = 60
global VAR #MQTT

# robot data
radius_wheel = 71/2
wheel_distance = 178
scope_wheel = radius_wheel*2*math.pi
gear = 3.2
turn_circle = wheel_distance*math.pi
    

# camera init
camera_columns = 480
camera_rows = 800
camera = PiCamera()
camera.framerate = 10
camera.resolution=(camera_rows,camera_columns)

# returns the proportion of green pixels in the taken photo
def traffic_light_detection():
    #grabbing a picture and converting it to an array in hsv color space
    raw_capture = PiRGBArray(camera, size=(camera_rows,camera_columns))
    camera.capture(raw_capture, format="bgr")
    image = raw_capture.array
    hsv_image = cv2.cvtColor(image,cv2.COLOR_BGR2HSV)
    
    #the v value corridor has been determined by evaluating the hsv_image matrix of an example image
    a = np.logical_and(hsv_image[:,:,0] > 45, hsv_image[:,:,0] < 60)

    #counting the green pixels of the picture
    green = np.count_nonzero(a) 

    #determining the proportion of the green pixles on the whole picture
    g = green / (camera_rows*camera_columns)
    print(g)
    return g

#functions returns data of detected barcode, else 0  
def barcodescanner():
    raw_capture = PiRGBArray(camera, size=(camera_rows,camera_columns))
    # grab an image from the camera
    camera.capture(raw_capture, format="bgr")
    image = raw_capture.array
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    scanner = zbar.Scanner()
    results = scanner.scan(image)
    raw_capture.truncate(0)
    if len(results) == 0:
        return 0
    else:
        for result in results:
            return result.data.decode("utf-8")

#function turn robot by selected angle
def turn(BP,degree):
    #clockwise
    #variables
    factor = 41/50 
    temp_r = BP.get_motor_encoder(BP.PORT_A)
    #calculation
    deg_right = -(gear*turn_circle*degree/scope_wheel)*factor
    deg_left = - deg_right
    #turning
    BP.set_motor_position_relative(BP.PORT_A, deg_right)
    BP.set_motor_position_relative(BP.PORT_D, deg_left)
    #checking if target value is reached
    while abs(BP.get_motor_encoder(BP.PORT_A)-temp_r) < abs(deg_right)-5:
        time.sleep(0.02)

#function lets robot drive a certain distance forwards
def driveforward_mm(BP,distance):
    #variables
    factor = 0.96
    #temp_r = BP.get_motor_encoder(BP.PORT_A)
    delta_angle = distance/scope_wheel*360*gear*factor
    #driving
    BP.set_motor_position_relative(BP.PORT_A, delta_angle)
    BP.set_motor_position_relative(BP.PORT_D, delta_angle)
    #checking if target value is reached
    time.sleep(0.5)
    while BP.get_motor_status(BP.PORT_A)[3] != 0:
        time.sleep(0.02)
    
#returns x value of the middle of the barcode; 0 means middle
def qr_position():
#function return the x-position of the middle of the barcode
    rawCapture = PiRGBArray(camera, size=(camera_rows,camera_columns))
    for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        image = frame.array
        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        scanner = zbar.Scanner()
        #scanner.scan return multiple information about the barcode
        results = scanner.scan(image)
        rawCapture.truncate(0)
        if len(results) == 0:
            #when no barcode is detected the function returns 0
            return 0
        else:
            #results contains one element per detected barcode
            for result in results:
                ans = result.position
                x=0
                for i in range(3):
                    #calculates the deviation using the positions of the barcodes corners
                    x = x + ans[i][0] - camera_rows/2
                x=x/4
                #also returns 0 when the badcode is perfectly central in front of the camera
                return x
        
def mqtt_pub(text,broker,topic):
    # broker is the IP of broker in the (internal) network
    #text is the message to send
    #topic should be raspiX with X beeing the number of the used robot
    publish.single(topic, text, hostname=broker)
  

def setup_client(topic, broker):    
    # The callback for when the client receives a CONNACK response from the server.
    def on_connect(client, userdata, flags, rc):
        client.subscribe(topic)
     
    # The callback for when a PUBLISH message is received from the server.
    def on_message(client, userdata, msg):
        global VAR
        VAR = str(msg.payload)
     
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(broker, 1883, 60)
    
    return client

#reads messages from referred broker and topic
def read_mqtt(broker,topic):
    # broker is the IP of broker in the (internal) network
    #topic should be raspiX with X beeing the number of the used robot
    global VAR
    VAR = None

    client = setup_client(topic, broker)

    while VAR == None:
        rc = client.loop()
    VAR = VAR[2:][:-1]
    return(VAR)

def shape_detection(form):
    #picture in high quality is taken and saved to Desktop to be able to controll the field of view
    camera.capture('/home/pi/Desktop/pic.jpg')
    #convert image to grayscale
    img = cv2.imread("/home/pi/Desktop/pic.jpg", cv2.IMREAD_GRAYSCALE)
    #create picture is just white and black based on predefined threshold (here: 80)
    unknown_threshold, threshold = cv2.threshold(img, 80, 255, cv2.THRESH_BINARY)
    contours, unknown_contours = cv2.findContours(threshold, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    strings = list()
    counter = 0

    for cnt in contours:
    #form is determined using the number of corners of the form
        approx = cv2.approxPolyDP(cnt, 0.01 * cv2.arcLength(cnt, True), True)

        if len(approx) == 3:
            strings.append('Triangle')

        elif len(approx) == 4:
            strings.append('Rectangle')

        elif len(approx) == 5:
            strings.append('Pentagon')

        elif 6 < len(approx) < 15:
            strings.append('Ellipse')

        else:
            strings.append('Circle')
    print("found forms:")
    length = len(strings)
    strings2 = strings[1:length]
    for j in strings2:
        print(j)
        if j == form:
            counter = counter + 1
    print("selected objects: ", counter)
    #counter contains the number of found shapes of the target form
    return counter

#function takes up a box
def lift_klt(BP):
    #down --> drive forwards --> up --> drive backwards
    BP.set_motor_position_relative(BP.PORT_B, -900)
    time.sleep(0.5)
    while BP.get_motor_status(BP.PORT_B)[3] != 0:
        time.sleep(0.02)
    driveforward_mm(BP,80)
    BP.set_motor_position_relative(BP.PORT_B, 900)
    time.sleep(0.5)
    while BP.get_motor_status(BP.PORT_B)[3] != 0:
        time.sleep(0.02)
    driveforward_mm(BP,-80)

#function takes down a box
def drop_klt(BP):
    #drive forwards --> down --> drive backwards --> up
    driveforward_mm(BP,80)
    BP.set_motor_position_relative(BP.PORT_B, -900)
    time.sleep(0.5)
    while BP.get_motor_status(BP.PORT_B)[3] != 0:
        time.sleep(0.02)
    driveforward_mm(BP,-80)
    BP.set_motor_position_relative(BP.PORT_B, 900)
    time.sleep(0.5)
    while BP.get_motor_status(BP.PORT_B)[3] != 0:
        time.sleep(0.02)

#function returns the pathcolor for the node given as input
def find_pathcolor(target_node):    
    #Here the pathcolors have to be assigned manually
    #so that the network can be driven off correctly
    color_dict = {'a':2, 'b':5, 'c':2, 'd':2, 'e':5,
                  'f':4, 'g':2, 'h':4, 'i':5, 'j':5,
                  'k':2, 'l':2, 'm':5, 'n':4, 'o':5, 'klt':3}
    path_color = color_dict[target_node]
    return path_color
        
#function to calculate the driven distance of both wheels
def calc_distance(enc_right, enc_left, enc_right_prev, enc_left_prev):
    #data needed for calculation
    radius_wheel = 71/2
    wheel_distance = 178
    scope_wheel = radius_wheel*2*math.pi
    gear = -3.2
    factor = 0.96
    #Encoderdifference to last step
    encdelta_right = enc_right_prev - enc_right
    encdelta_left = enc_left_prev - enc_left
    #calculating driven distance
    dis_right = encdelta_right*scope_wheel/(360*factor*gear)
    dis_left = encdelta_left*scope_wheel/(360*factor*gear)
    return [dis_left, dis_right]


#from Github:
#calculates the position-, angle- and distancedelta of the robot
#needs the driven distance of both wheels as well as the distance between the wheels and the start-angle
def get_position_delta(distance_left, distance_right, previous_angle, wheel_distance, curve_adjustment=1):
    # takes the distance driven by the left and right wheel (and their distance)
    # and a starting angle, calculates new position and orientation

    # bunch of maths, look at the readme if you care

    # straight movement or curve?
    if distance_right == distance_left:
        # straight movement
        delta_angle = 0
        delta_distance = abs(distance_left)
        delta_x = distance_left * math.sin(previous_angle)
        delta_y = distance_left * math.cos(previous_angle)
    else:
        # curve movement

        # curve distance adjustments. look at readme
        try:
            relation = distance_left / distance_right
        except ZeroDivisionError:
            relation = 0

        if relation > 0.5 and relation < 2:
            factor = 1
        else:
            factor = curve_adjustment

        distance_left *= factor
        distance_right *= factor

        distance_difference = distance_left - distance_right
        radius_left = (distance_left * wheel_distance) / distance_difference
        radius_right = (distance_right * wheel_distance) / distance_difference

        try:
            angle = distance_left / radius_left
        except ZeroDivisionError:
            angle = distance_right / radius_right

        radius = (radius_left + radius_right) / 2
        delta_angle = angle
        euclidean_distance = math.sqrt(2 * radius * radius * (1 - math.cos(angle)))
        # as defined by readme
        angle_lambda = ((math.pi - angle) / 2) - previous_angle
        delta_x = euclidean_distance * math.cos(angle_lambda)
        delta_y = euclidean_distance * math.sin(angle_lambda)
        delta_distance = abs(radius * angle)

    # build return data
    delta_dict = {
        'delta_angle': delta_angle,
        'delta_distance': delta_distance,
        'delta_x': delta_x,
        'delta_y': delta_y,
    }
    return delta_dict
