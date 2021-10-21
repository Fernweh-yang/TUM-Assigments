#Import libraries
import time
import brickpi3
import follower
import functions

def main(BP):
    
    while True:
        stop =follower.line_follower(BP,300,7,0.01,[5])
        if stop == "Obstacle":
            print("Warte 5 sek")
            time.sleep(5)
            if BP.get_sensor(BP.PORT_1) <= 15:
                print("Umfahre Hindernis")
                avoid_obst(BP)
                continue
            else:
                print("Fahre weiter")
        else:
            break
   
def avoid_obst(BP):
    #Turn around so that the us-sensor on the side is facing the obstacle
    functions.turn(BP, -80)
    #Drive forward, until the distance to the obstacle is smaller than 15cm
    while BP.get_sensor(BP.PORT_2) < 10:
        BP.set_motor_dps(BP.PORT_A, 300)
        BP.set_motor_dps(BP.PORT_D, 300)
    #Start distance follower
    stop = follower.distance_follower(BP, 250, 8, 0.01)
    #If the line is reached again -> Start linefollower
    if stop is "Black":
        print('Line found')
        #Turn, so that the robot faces the right direction again
        functions.driveforward_mm(BP, 80)
        functions.turn(BP,-65)
    print("Erfolgreich Umfahren")
    return 0
        
