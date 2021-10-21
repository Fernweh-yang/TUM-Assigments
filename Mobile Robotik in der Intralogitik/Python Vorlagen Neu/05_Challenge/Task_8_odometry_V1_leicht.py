#Import libraries
import brickpi3
import functions
import time
import math
import follower
    
def main(BP):

    dist_x = 1060
    dist_y = 1000
    x = math.sqrt(dist_x**2+dist_y**2)
    alpha = math.atan(dist_x/dist_y)/math.pi*180*0.97
    follower.line_follower(BP,250,7,0.01,[2])
    functions.driveforward_mm(BP, 120)
    
    
    functions.turn(BP, -alpha)
    time.sleep(1)
    fahren_bis(BP, x)
    #functions.driveforward_mm(BP, x*10)
    
def fahren_bis(BP, dist):
    #variables
    factor = 0.96
    radius_wheel = 71/2
    wheel_distance = 178
    scope_wheel = radius_wheel*2*math.pi
    gear = 3.2
    #temp_r = BP.get_motor_encoder(BP.PORT_A)
    delta_angle = dist/scope_wheel*360*gear*factor
    BP.offset_motor_encoder(BP.PORT_A, BP.get_motor_encoder(BP.PORT_A))
    BP.offset_motor_encoder(BP.PORT_D, BP.get_motor_encoder(BP.PORT_D))
    delta_gemessen = 0
    print("Motor Encode am Anfang:", BP.get_motor_encoder(BP.PORT_A))
#     timestep = 0.001
    while delta_gemessen < delta_angle:
        obst = BP.get_sensor(BP.PORT_1) < 10
        delta_gemessen = BP.get_motor_encoder(BP.PORT_A)
        time.sleep(0.01)
        if obst:
            BP.set_motor_dps(BP.PORT_A, 0)
            BP.set_motor_dps(BP.PORT_D, 0)
            print("delta_gemessen:", delta_gemessen)
            print("Delta noch zu fahren:", delta_angle-delta_gemessen)
            time.sleep(1)
            continue
        else:
            BP.set_motor_dps(BP.PORT_A, 400)
            BP.set_motor_dps(BP.PORT_D, 400)
    BP.set_motor_dps(BP.PORT_A, 0)
    BP.set_motor_dps(BP.PORT_D, 0)
            
        
            