import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import time
import numpy as np
import brickpi3

global VAR 
         
def mqtt_pub(text,broker,topic):                                    # Funktion zum Versenden von Nachrichten an Client
    MQTT_SERVER = "192.168.0.99"                                                 # IP-Adresse des MQTT Servers als String eingeben  
    MQTT_PATH = "Gruppe_5-Farbe_erkennen"                                                    # Bennenung des Topics nach dem Farberkennung - Gruppenname, als String einfügen
    publish.single(MQTT_PATH, text, hostname=MQTT_SERVER)

def setup_client(MQTT_PATH, MQTT_SERVER):    
    def on_connect(client, userdata, flags, rc):
        client.subscribe(MQTT_PATH)
     
    def on_message(client, userdata, msg):
        global VAR
        VAR = str(msg.payload)
             
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_SERVER, 1883, 60)              #IP-Adresse des MQTT Servers als String eingeben
    return client

def read_mqtt(MQTT_SERVER,MQTT_PATH):
    global VAR
    VAR = None

    client = setup_client(MQTT_PATH, MQTT_SERVER)    #IP-Adresse des MQTT Servers als String eingeben

    while VAR == None:
        rc = client.loop()
    
    return(VAR[2:][:-1])


BP=brickpi3.BrickPi3()

BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_COLOR)# Wählen des richtigen Ports und BrickPi3 Befehls für Farbsensoren

color = ["none", "Black", "Blue", "Green", "Yellow", "Red", "White", "Brown"]
color_check = []
final_color = ""

#Einfügen der Lösung aus Aufgabe 2-1
#Initialieren des Sensors
#Zuweisen von Farben an die Sensorwerte

try:
    while True:                         
        msg = None 
        msg = read_mqtt("192.168.0.99","Gruppe_5-Farbe_erkennen")       #Name des Topics und IP-Adresse des MQTT Servers als String eingeben
        print(msg)
        time.sleep(1)
        if msg == "Befehl zum Farben Erkennen":
            for i in range(1,4):
                valueA = BP.get_sensor(BP.PORT_3)
                color_check.append(valueA)
                time.sleep(1)
            
            color_set = set(color_check)
            
            if len(color_set)==1:                                             # Ausgabe der Farbe nur bei Übereinstimmung
                final_color=color[color_check[0]]              # Angabe der Farbname aus dem color-Array bei der richtigen Erkennung der Farben
                print(final_color)
            # Funktion für das Senden einer Nachricht, z.B. der erkannten Farbe, an Client: 
                mqtt_pub("Farbe erkannt: "+final_color,"192.168.0.99","Gruppe_5-Farbe_erkennen")
            else:
                print("Farbe nicht sicher erkannt!")
                mqtt_pub("Farbe nicht sicher erkannt","192.168.0.99","Gruppe_5-Farbe_erkennen")     #Name des Topics und IP-Adresse des MQTT Servers als String eingeben
                time.sleep(2)
            
            color_check.clear()
            color_set.clear()
            
#         elif  msg != "richtig erkannt":
#             msg = read_mqtt("192.168.0.99","Gruppe_5-Farbe_erkennen")                               #Name des Topics und IP-Adresse des MQTT Servers als String eingeben
#             print(msg)
#             time.sleep(1)
            
        else:
            print("Kein Message")

except KeyboardInterrupt:
    BP.reset_all()