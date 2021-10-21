#Import libraries
import brickpi3
import follower
from math import pi, sin, cos, atan, sqrt
import functions
import time
    
def main(BP):
    #Set Start- and endpoint
    start = []
    end = []
    stop = follower.line_follower(BP,120,1,0.001,[2])

#Follow the calculated, imaginary line
    #Calculate the starting angle
    angle = atan((end[1]-start[1])/(end[0]-start[0]))
    angle_deg = angle*180/pi
    print('Starting Angle: ' + str(angle_deg))
    #Calculate the distance to the endpoint
    distance = sqrt((end[1]-start[1])**2 + (end[0]-start[0])**2)
    print('Distance: ' + str(distance))
    #Turn into the right direction
    functions.turn(BP,-angle_deg)
    
    #Set encoder and startposition
    enc_right_prev = BP.get_motor_encoder(BP.PORT_A)
    enc_left_prev = BP.get_motor_encoder(BP.PORT_D)
    x = start[0]
    y = start[1]
    
    #While the endpoint is not reached -> Drive
    endpoint = False
    BP.set_motor_dps(BP.PORT_A, 300)
    BP.set_motor_dps(BP.PORT_D, 300)
    while endpoint is False:            
        time.sleep(0.1)
        try:
            
            # ... your code for recognize obstacle...
            
            
            
            
        except brickpi3.SensorError as error:
            print (error)
                    
    #ODOMETRY#                        
        #Read encoders


        #Calculate the distance
  
  
        #Odometry for straight movement


        #Set variables for the next iteration
        
        
        #Check, if endpoint is reached


        #Error of 5mm is allowed


        
    print('Made it! Coordinates: ' + str(x) + ' ' + str(y))
