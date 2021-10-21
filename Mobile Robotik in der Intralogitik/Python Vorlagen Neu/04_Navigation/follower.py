import time
import brickpi3
import functions

#####################################        
# LINE FOLLOWER
def line_follower(BP,velocity,Kp,Ki,stop_colours):
    stop_reason = 0
    #If stop_colours is empty, the robot should stop at blue, green, yellow, red and brown
    if not stop_colours:
        stop_colours = [2,3,4,5,7]
        
    while True:
        try:
            # Control inits
            speed = velocity
            time_cur = time.time()
            time_last = time.time()
            integral = 0
            previous_error = 0

            # Reflection target value for target position
            target_value = 30

            # Main Loop
            while True:
                
                # Get color of left sensor
                color_left = BP.get_sensor(BP.PORT_4)
                # If color is stop_colour -> Set stop_reason to 1 and leave loop
                for colour in stop_colours:
                    if (color_left is colour):
                        if color_left == 2:
                            print('Doublecheck blue marker')
                            functions.driveforward_mm(BP,3)
                            color_left = BP.get_sensor(BP.PORT_4)
                            if (color_left is colour):
                                stop_reason = 1
                                break
                        else:
                            stop_reason =1
                            break
                if stop_reason == 1:
                    break
                # Get distance of ultrasonic sensor
                distance_front = BP.get_sensor(BP.PORT_1)
                # If distance is less than 10 cm -> Set stop_reason to 2
                if distance_front <= 12 and distance_front != 0:
                    stop_reason = 2
                    break

                # Get reflected light of right sensor
                cs = BP.get_sensor(BP.PORT_3)
                # Time delta since last loop
                time_last = time_cur
                time_cur = time.time()
                dt = time_cur - time_last
                
                # Calculate steering using PI algorithm
                error = target_value - cs
                integral += (error * dt)
                u = (Kp * error) + (Ki * integral)

                # Limit u to 500
                if speed + abs(u) > 500:
                    if u >= 0:
                        u = 500 - speed
                    else:
                        u = speed - 500

                # Run motors
                if u >= 0:
                    BP.set_motor_dps(BP.PORT_A,speed - abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed + abs(u))
                    time.sleep(0.001)
                else:
                    BP.set_motor_dps(BP.PORT_A,speed + abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed - abs(u))
                    time.sleep(0.001)
                previous_error = error

        except brickpi3.SensorError as error:
            print (error)
        
        # Stop robot, when leaving the loop
        BP.set_motor_dps(BP.PORT_A,0)
        BP.set_motor_dps(BP.PORT_D,0)
        
        # If stopped due to color
        if stop_reason is 1:
            if color_left is 1:
                return('Black')
            if color_left is 2:
                return('Blue')
            if color_left is 3:
                return('Green')
            if color_left is 4:
                return('Yellow')
            if color_left is 5:
                return('Red')
            if color_left is 6:
                return('White')
            if color_left is 7:
                return('Brown')
            print('Farbe erkannt: ' + str(color_left))
            break
        
        # If stopped due to obstacle
        if stop_reason is 2:
            print('Obstacle in ' + str(distance_front) + 'cm distance!')
            return('Obstacle')

#############################################
# DISTANCE FOLLOWER
def distance_follower (BP,velocity,Kp,Ki):    

    # Setting right colorsensor to color-mode
    BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_COLOR)
    time.sleep(1)

    stop_reason = 0   
    while True:
        try:
            # Control inits
            speed = velocity
            time_cur = time.time()
            time_last = time.time()
            integral = 0
            previous_error = 0

            # Distance target value (in cm)
            target_value = 12

            # Main Loop
            while True:
                
                # Get color of right sensor
                color_right = BP.get_sensor(BP.PORT_3)
                # If color is black -> Set stop_reason to 1
                if (color_right is 1):
                    stop_reason = 1
                    break

                # Getting distance (side)
                cs = BP.get_sensor(BP.PORT_2)
                # Time delta since last loop
                time_last = time_cur
                time_cur = time.time()
                dt = time_cur - time_last
                
                # Calculate steering using PID algorithm
                error = target_value - cs
                integral += (error * dt)
                u = (Kp * error) + (Ki * integral)

                #limit u to 500
                if speed + abs(u) > 500:
                    if u >= 0:
                        u = 500 - speed
                    else:
                        u = speed - 500

                # Run motors. Watch out, u is the other way!
                if u <= 0:
                    BP.set_motor_dps(BP.PORT_A,speed - abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed + abs(u))
                    time.sleep(0.001)
                else:
                    BP.set_motor_dps(BP.PORT_A,speed + abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed - abs(u))
                    time.sleep(0.001)
                previous_error = error

        except brickpi3.SensorError as error:
            print (error)
        
        # Stop motors, when leaving the loop
        BP.set_motor_dps(BP.PORT_A,0)
        BP.set_motor_dps(BP.PORT_D,0)
        BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_REFLECTED)
        
        # If stopped due black line
        if stop_reason is 1:
            return('Black')
            break

#########################################################################
# DISTANCE FOLLOWER WITH ODOMETRY (NOT USED IN THE CHALLENGE)
def distance_follower_odometry(BP,velocity,Kp,Ki,angle,x,y,start,end):    

    # Setting right colorsensor to color-mode
    BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_COLOR)
    
    stop_reason = 0       
    i = 0
    # Calc m
    m = (end[1]-start[1])/(end[0]-start[0])
    # Calc t
    t = start[1] - start[0]*m    
    # Set encoder
    enc_right_prev = BP.get_motor_encoder(BP.PORT_A)
    enc_left_prev = BP.get_motor_encoder(BP.PORT_D)
    
    while True:
        try:
            # Control inits
            speed = velocity
            time_cur = time.time()
            time_last = time.time()
            integral = 0
            previous_error = 0

            # Distance target value (in cm)
            target_value = 8

            # Main Loop
            while True:
                
                # Loop counter
                i = i+1
                # Calc deviation from imaginary line
                linecheck = abs((m*x) + t - y)
                print('x,y,linecheck: ', str(x),str(y),str(linecheck))
                
                # If deviation less than 5 mm and counter bigger than 100 -> Set stop_reason to 1
                if (linecheck <= 2) and (i>= 200):
                    stop_reason = 1
                    print('Stopped!')
                    break

                # Get distance (cm)
                cs = BP.get_sensor(BP.PORT_2)
                # Time delta since last loop
                time_last = time_cur
                time_cur = time.time()
                dt = time_cur - time_last
                
                # Calculate steering using PID algorithm
                error = target_value - cs
                integral += (error * dt)
                u = (Kp * error) + (Ki * integral)

                # Limit u to 500
                if speed + abs(u) > 500:
                    if u >= 0:
                        u = 500 - speed
                    else:
                        u = speed - 500

                # Run motors. Watch out, u is the other way!
                if u <= 0:
                    BP.set_motor_dps(BP.PORT_A,speed - abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed + abs(u))
                    #time.sleep(0.001)
                else:
                    BP.set_motor_dps(BP.PORT_A,speed + abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed - abs(u))
                    #time.sleep(0.001)
                previous_error = error
                
            # ODOMETRY                        
                # Read encoder values
                enc_right = BP.get_motor_encoder(BP.PORT_A)
                enc_left = BP.get_motor_encoder(BP.PORT_D)
                # Calculate driven distance of both wheels
                [dis_left,dis_right] = functions.calc_distance(enc_right, enc_left, enc_right_prev, enc_left_prev)
                # Calculate delta
                delta_dict = functions.get_position_delta(dis_left,dis_right,angle,170,1)
                # Save variables of this iteration for the next one
                enc_right_prev = enc_right
                enc_left_prev = enc_left
                angle = angle + delta_dict['delta_angle']
                x = x + delta_dict['delta_x']
                y = y + delta_dict['delta_y']                                             

        except brickpi3.SensorError as error:
            print (error)
        
        # Stop motors when leaving loop
        BP.set_motor_dps(BP.PORT_A,0)
        BP.set_motor_dps(BP.PORT_D,0)
        BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_REFLECTED)
        
        # If stopped due finding the imaginary line
        if stop_reason is 1: 
            print('Found imaginary line')
            return('Line')
            break

######################################
# QR FOLLOWER
def QR_follower(BP,velocity,Kp,Ki):
    stop_reason = 0
    while True:
        try:
            # Control inits
            speed = velocity
            time_cur = time.time()
            time_last = time.time()
            integral = 0
            previous_error = 0

            # Target value for horizontal deviation
            target_value = 0

            # Main Loop
            while True:
                
                # Get color of left sensor
                color_left = BP.get_sensor(BP.PORT_4)
                # If color is Red -> Set stop_reason to 1
                if color_left is 5:
                    stop_reason = 1
                    print('Red')
                    break
                
                # Getting deviation
                deviation = functions.qr_position()
                print(deviation)
                # Time delta since last loop
                time_last = time_cur
                time_cur = time.time()
                dt = time_cur - time_last
                # Scale the control-value                                              
                cs = deviation/10
                
                # Calculate steering using PID algorithm
                error = target_value - cs
                integral += (error * dt)
                u = (Kp * error) + (Ki * integral)

                # Limit u to 500
                if speed + abs(u) > 500:
                    if u >= 0:
                        u = 500 - speed
                    else:
                        u = speed - 500

                # Run motors
                if u >= 0:
                    BP.set_motor_dps(BP.PORT_A,speed - abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed + abs(u))
                    time.sleep(0.01)
                else:
                    BP.set_motor_dps(BP.PORT_A,speed + abs(u))
                    BP.set_motor_dps(BP.PORT_D,speed - abs(u))
                    time.sleep(0.01)
                previous_error = error

        except brickpi3.SensorError as error:
            print (error)
        
        # Stop motors when leaving loop
        BP.set_motor_dps(BP.PORT_A,0)
        BP.set_motor_dps(BP.PORT_D,0)
        
        # If stopped due red marker
        if stop_reason is 1:
            return('Red')
            break
