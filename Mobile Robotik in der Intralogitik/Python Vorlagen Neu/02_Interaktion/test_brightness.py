import brickpi3
import time

#####################################
# Funktion: LINE FOLLOWER (Start)
#####################################
def line_follower(BP,velocity,Kp):
    while True:
        BP.set_motor_dps(BP.PORT_A, velocity)
        BP.set_motor_dps(BP.PORT_D, velocity)
        # Control inits
        
        while True:
            speed_right = 0
            speed_left = 0
            # Get reflected light of right sensor
            sensor_val = BP.get_sensor(BP.PORT_3)
            print("Sensor Val:", sensor_val)
            
            # Calculate steering using P algorithm
            steer_val = (30-sensor_val)*Kp
            print("Steer Val:", steer_val)

            # Limit u to 500
            max_speed = 500
            
            if velocity > max_speed:
                velocity = max_speed
            
            if steer_val < 0:
                speed_right = velocity - abs(steer_val)
                BP.set_motor_dps(BP.PORT_A, speed_right)
                print("Right Motor Val:", speed_right)
            elif steer_val > 0 :
                speed_left = velocity - abs(steer_val)
                BP.set_motor_dps(BP.PORT_D, speed_left)
                print("Left Motor Val:", speed_left)
            time.sleep(0.05)   
            

            # Run motors

#####################################
# Funktion: LINE FOLLOWER (Ende)
#####################################


#####################################
# Hauptprogramm: Start
#####################################

# Initialisierungen
BP = brickpi3.BrickPi3()
BP.set_sensor_type(BP.PORT_1, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM)
BP.set_sensor_type(BP.PORT_2, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM)
BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_REFLECTED)
BP.set_sensor_type(BP.PORT_4, BP.SENSOR_TYPE.EV3_COLOR_COLOR)
time.sleep(3)

try:
    # Aufruf der oben definierten Funktion
    line_follower(BP, 200, 5)
except KeyboardInterrupt:
    BP.reset_all()

#####################################
# Hauptprogramm: Ende
#####################################
