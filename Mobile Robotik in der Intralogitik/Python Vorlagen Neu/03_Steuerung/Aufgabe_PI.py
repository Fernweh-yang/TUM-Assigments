import time
import brickpi3

#####################################
# Funktion: LINE FOLLOWER (Start)
#####################################
def line_follower(BP,velocity,Kp,Ki):
    while True:
        # Control inits

        # Reflection target value for target position

        # Main Loop
        # Control inits
        BP.set_motor_dps(BP.PORT_A, velocity)
        BP.set_motor_dps(BP.PORT_D, velocity)
        fehler_int = 0
        last_t = time.time()
        # Reflection target value for target position
        

        # Main Loop
        while True:
            speed_right = 0
            speed_left = 0
            
            # Get reflected light of right sensor
            sensor_val = 30
            try:
                sensor_val = BP.get_sensor(BP.PORT_3)
            except:
                print("Sensor error")
                
            
            delta_t = time.time() - last_t
            fehler_int = fehler_int + delta_t * (30 - sensor_val)
            # Calculate steering using P algorithm
            steer_val = (30 - sensor_val) * Kp + Ki * fehler_int

            # Limit u to 500
            max_speed = 500
            
            if velocity > max_speed:
                velocity = max_speed
            
            if steer_val < 0:
                speed_right = velocity - abs(steer_val)
                BP.set_motor_dps(BP.PORT_A, speed_right)
            elif steer_val > 0 :
                speed_left = velocity - abs(steer_val)
                BP.set_motor_dps(BP.PORT_D, speed_left)
            last_t = time.time()
            time.sleep(0.01)   
            
            # Get reflected light of right sensor

            # Time delta since last loop
            
            # Calculate steering using PI algorithm

            # Limit u to 500

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
    line_follower(BP, 400, 8, 0.4)
except KeyboardInterrupt:
    BP.reset_all()

#####################################
# Hauptprogramm: Ende
#####################################