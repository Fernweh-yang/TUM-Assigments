#Import Libraries
import functions
import time
import brickpi3
import follower

def main(BP):

    BROKER = "192.168.0.99"
    TOPIC = "raspi5"
    stop = follower.line_follower(BP,300,7,0.01,[2])
    functions.mqtt_pub("Angekommen", BROKER, TOPIC)
    print("Angekommen published")
    #BP.set_motor_position_relative(BP.PORT_B, 900)
    time.sleep(0.5)
    found = False
    forms = ["Circle", "Rectangle", "Triangle", "Ellipse", "Tentagon"]
    while not found:
        msg = functions.read_mqtt(BROKER, TOPIC)
        print("Msg gelesen:", msg)
        for i in forms:
            if msg in i:
                found =  True
                print("Nachricht ist enthalten")
                break
            else:
                print("Form nicht enthalten")
    print("Weiter")
    stop = follower.line_follower(BP,150,7,0.01,[5])
    return msg