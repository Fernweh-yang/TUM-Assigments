import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import time
import numpy as np
import brickpi3

## Variables
MQTT_SERVER ="192.168.0.99"                    # IP-Adresse des MQTT Servers als String eingeben  
MQTT_TOPIC = "valentin/testmqtt"                    # Bennenung des Topics nach dem Farberkennung - Gruppenname, als String einfügen



######## MQTT STUFF #########
global VAR # Die Variable wird von den MQTT funktionen genutzt. 
def mqtt_pub(text):                                    # Funktion zum Versenden von Nachrichten an Client zum default topic
    publish.single(MQTT_TOPIC, text, hostname=MQTT_SERVER)

def setup_client(MQTT_PATH, MQTT_SERVER):    
    def on_connect(client, userdata, flags, rc):
        client.subscribe(MQTT_TOPIC)
     
    def on_message(client, userdata, msg):
        global VAR
        VAR = str(msg.payload)
             
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_SERVER, 1883, 60)              
    return client

def read_mqtt(client):
    global VAR
    VAR = None

    while VAR == None:
        rc = client.loop()
    
    return(VAR[2:][:-1])



# init BrickPi and Mqtt Client 
BP=brickpi3.BrickPi3()
mqtt_client = setup_client(MQTT_TOPIC, MQTT_SERVER)

#Initialieren des Sensors
#Zuweisen von Farben an die Sensorwerte

try:
    while True:                         
        msg = None
        msg = read_mqtt(mqtt_client)       #Blocking (Die read_mqtt function läuft solange bis eine message erkannt wird)
        print(msg)
        if msg == "farbe":
            # Einarbeiten der Lösung aus Aufgabe 2-1
            farbe_sicher_erkannt = False
            
            if farbe_sicher_erkannt:
                print("FARBE SICHER EKEANNT")
            # Funktion für das Senden einer Nachricht, z.B. der erkannten Farbe, an Client: 
            # mqtt_pub("Farbe erkannt: " + color[valueB])
            else:
                print("Farbe net sicher erkannt")
                mqtt_pub("testobesklappt")
                ## Was passiert wenn die Farbe nicht sicher erkannt wurde ?
               
               
        if msg == "richtig erkannt":
            mqtt_pub("ICH WEISS ZEFIX")
            print("Yuhuu")

except KeyboardInterrupt:
    BP.reset_all()