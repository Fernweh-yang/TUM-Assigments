import time
import brickpi3
import follower
import functions

try:
    BP = brickpi3.BrickPi3()
    BP.set_motor_limits(BP.PORT_A, dps = 200)
    BP.set_motor_limits(BP.PORT_D, dps = 200)
    BP.set_motor_limits(BP.PORT_B, dps = 400)
    BP.set_sensor_type(BP.PORT_1, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM)
    BP.set_sensor_type(BP.PORT_2, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM)
    BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_REFLECTED)
    BP.set_sensor_type(BP.PORT_4, BP.SENSOR_TYPE.EV3_COLOR_COLOR)
    time.sleep(4)

    def main(BP):
        while True:
            #Check for obstacle
            BP.set_motor_dps(BP.PORT_A, 400)
            BP.set_motor_dps(BP.PORT_D, 400)
            if BP.get_sensor(BP.PORT_1) <= 12:
                print('Recognized Obstacle!')
                #bypass obstacle
                print('Bypass the obstacle')
                avoid_obst(BP)

    def avoid_obst(BP):
        #Turn around so that the us-sensor on the side is facing the obstacle
        functions.turn(BP, -80)
        #Drive forward, until the distance to the obstacle is smaller than 15cm
        while BP.get_sensor(BP.PORT_2) < 15:
            BP.set_motor_dps(BP.PORT_A, 300)
            BP.set_motor_dps(BP.PORT_D, 300)
        #Start distance follower
        stop = follower.distance_follower(BP, 400, 8, 0.01)
        print(stop)
        #If the line is reached again -> Start linefollower
        if stop is "Black":
            print('Line found')
            #Turn, so that the robot faces the right direction again
            functions.driveforward_mm(BP, 50)
            functions.turn(BP,-90)
            
        
    main(BP)

except KeyboardInterrupt:
     BP.reset_all()

