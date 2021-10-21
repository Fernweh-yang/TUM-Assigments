import time
import brickpi3 

BP = brickpi3.BrickPi3()
BP.set_motor_limits(BP.PORT_B, 50, 300)                             # Limits der Motorleistung und der Umdrehungen AUF KEINEN FALL verändern

																	# (2) HERE function to set encoder to ZERO (once --> set as comment afterwards)
try:

    try:
																	# (1) HERE function to reset encoder with the current position
    except IOError as error:
        print(error)
    
    while True:
        try:
            print(...)                                              # HERE function to read motor position
        except IOError as error:                                    # Handled Fehler, falls welche auftreten
            print(error)
        
        # HERE function for setting motor target position. Zur Sicherheit Inkremente von MAXIMAL 360 wählen
        time.sleep(0.02)                                            # delay for 0.02 seconds (20ms) to reduce the Raspberry Pi CPU load.

except KeyboardInterrupt:
    BP.reset_all()