import time     
import brickpi3 

BP = brickpi3.BrickPi3() # Create an instance of the BrickPi3 class. BP will be the BrickPi3 object.

BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_COLOR)# Wählen des richtigen Ports und BrickPi3 Befehls für Farbsensoren


color = ["none", "Black", "Blue", "Green", "Yellow", "Red", "White", "Brown"]
color_check=[]

try:
    while True:
        try:
        #Vergleich von nacheinander erkannten Farben
            for i in range(1,4):
                valueA = BP.get_sensor(BP.PORT_3)
                color_check.append(valueA)
                time.sleep(1)
            
            color_set = set(color_check)
            
            if len(color_set)==1:                                             # Ausgabe der Farbe nur bei Übereinstimmung
                print("Farbe erkannt: ", color[color_check[0]])               # Angabe der Farbname aus dem color-Array bei der richtigen Erkennung der Farben
            else:
                print("Farbe nicht sicher erkannt")
              
            color_check.clear()
            color_set.clear()
            
        except brickpi3.SensorError as error:
            print(error)
        
        time.sleep(0.02) 

except KeyboardInterrupt:                                     # except the program gets interrupted by Ctrl+C on the keyboard.
    BP.reset_all()                                            # Unconfigure the sensors, disable the motors, and restore the LED to the control of the BrickPi3 firmware.