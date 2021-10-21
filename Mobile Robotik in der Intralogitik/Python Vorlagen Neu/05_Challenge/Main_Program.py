import brickpi3
import time
import follower
import functions
import Task_1_traffic_light
import Task_2_KLT
import Task_3_network
import Task_4_MQTT
import Task_5_QR_follower
import Task_6_obstacle
import Task_7_shape_recognition
import Task_8_odometry_V1_leicht

# INITS
BP = brickpi3.BrickPi3()
BP.set_motor_limits(BP.PORT_A, dps = 300)
BP.set_motor_limits(BP.PORT_D, dps = 300)
BP.set_motor_limits(BP.PORT_B, dps = 400)
BP.set_sensor_type(BP.PORT_1, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM)
BP.set_sensor_type(BP.PORT_2, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM)
BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_REFLECTED)
BP.set_sensor_type(BP.PORT_4, BP.SENSOR_TYPE.EV3_COLOR_COLOR)
time.sleep(3)
target_form = 'Circle' #Init for Task7

# Main loop - start from anywhere!
try:
    while True:
        stop = follower.line_follower(BP,150,7,0.01,[5])
        # Follow the line to the next red marker
        if stop == 'Red':
            task_number = 0
            functions.turn(BP,-90)
            
            print('searching for task number')
            # Checking task number       
            while task_number == 0:
                task_number = int(functions.barcodescanner())
                time.sleep(0.01)
            print("recognized task: " + str(task_number))
            functions.turn(BP,90)

            #Ensures that only for valid tasks marker is passed
            for i in range(1,8,1):
                if task_number == i:
                    functions.driveforward_mm(BP,functions.marker_offset)
            # Jumping to task
            if task_number == 1:
                Task_1_traffic_light.main(BP)    
            elif task_number == 2:
                Task_2_KLT.main(BP)
            elif task_number == 3:
                 Task_3_network.main(BP)
            elif task_number == 4:
                target_form = Task_4_MQTT.main(BP)
            elif task_number == 5:
                Task_5_QR_follower.main(BP)
            elif task_number == 6:
                Task_6_obstacle.main(BP)
            elif task_number == 7:
                Task_7_shape_recognition.main(BP, target_form)
            elif task_number == 8:
                Task_8_odometry_V1_leicht.main(BP)
                break
            else:
                print("Error: This task does not exist. Continue LineFollower.")
                
        else:
            print("Error: Stop was not caused by a red marker. Continue LineFollower.")
            print(stop)
            
except KeyboardInterrupt:
     BP.reset_all() 
