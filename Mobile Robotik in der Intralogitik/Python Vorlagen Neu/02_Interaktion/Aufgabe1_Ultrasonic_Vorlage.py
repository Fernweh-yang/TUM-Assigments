import time     # import the time library for the sleep function
import brickpi3 # import the BrickPi3 drivers

def exceptSensorError(func,args):
    try:
        return func(args)
    except brickpi3.SensorError as error:
        print(error)

BP = brickpi3.BrickPi3() # Create an instance of the BrickPi3 class. BP will be the BrickPi3 object.

# vier wichtige Sensorfunktionen:
BP.set_sensor_type(BP.PORT_1, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM) # Configure for an EV3 ultrasonic sensor.
# BP.get_sensor(Port) retrieves a sensor value.
# BP.PORT_1 specifies that we are looking for the value of sensor port 1.
# BP.get_sensor returns the sensor value (what we want to display).

try:
    while True:
        value = exceptSensorError(BP.get_sensor,BP.PORT_1)
        #value = BP.get_sensor(BP.PORT_1)
        print(value)
        time.sleep(0.5)                         # delay for 0.5 seconds (20ms) to reduce the Raspberry Pi CPU load.

except KeyboardInterrupt:                       # except the program gets interrupted by Ctrl+C on the keyboard.
    BP.reset_all()                              # Unconfigure the sensors, disable the motors, and restore the LED to the control of the BrickPi3 firmware.