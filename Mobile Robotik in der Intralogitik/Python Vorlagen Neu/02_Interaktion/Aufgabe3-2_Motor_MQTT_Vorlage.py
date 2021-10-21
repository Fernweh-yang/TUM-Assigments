import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import time
import numpy as np
import brickpi3

global VAR 
         
def mqtt_pub(text,broker,topic):                                    # Funktion zum Versenden von Nachrichten an Client
    MQTT_SERVER = "192.168.0.99"                                                  # IP-Adresse des MQTT Servers als String eingeben  
    MQTT_PATH = "Gruppe_5"                                                    # Benennung des Topics nach dem "Hubwerk - Gruppenname", als String einfügen
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

    client.connect("192.168.0.99", 1883, 60)                        # IP-Adresse des MQTT Servers als String eingeben 
    return client

def read_mqtt(MQTT_SERVER,MQTT_PATH):
    global VAR
    VAR = None

    client = setup_client("Gruppe_5", "192.168.0.99")                # IP-Adresse des MQTT Servers als String eingeben 

    while VAR == None:
        rc = client.loop()
    
    return(VAR[2:][:-1])

BP=brickpi3.BrickPi3()
BP.set_motor_limits(BP.PORT_B, 60, 300)
mqtt_pub("Bereit","192.168.0.99","Gruppe_5")         # IP-Adresse des MQTT Servers und Benennung des Topics nach dem "Hubwerk - Gruppenname", als String einfügen

    
while True:                                             # Dauerhaftes Lesen der Nachrichten
    msg = read_mqtt("192.168.0.99","Gruppe_5")          # IP-Adresse des MQTT Servers und Benennung des Topics nach dem "Hubwerk - Gruppenname", als String einfügen
    try: 
        print(msg)
        if msg == 'Befehl zum Heben':                   # Stichwort, das via MQTT-fx eingegeben wird, z.B. 'Heben'
            token = True
            BP.set_motor_position(BP.PORT_B,600)
            # Implementieren einer Funktion zum Heben des Hubwerks, nachdem durch Stichwort via MQTT getriggert wird
            # Einarbeiten der Lösung aus Aufgabe 3-1
            
            mqtt_pub("Erfolgreich gehoben","192.168.0.99","Gruppe_5")
            # Funktion zum Versenden einer Nachricht zum Client, z.B. "Name des Befehls", "IP Adresse", "Gruppenname"
            # Siehe Aufgabe 2-2
            
            time.sleep(0.02)                             # delay for 0.02 seconds (20ms) to reduce the Raspberry Pi CPU load.
            token = False
            msg = None
        
        if msg == 'Befehl zum Senken':                   # Stichwort, das via MQTT-fx eingegeben wird, z.B. 'Senken'
            # Implementieren einer Funktion zum Senken des Hubwerks, nachdem durch Stichwort via MQTT getriggert wird
            # Analog zu 'Befehl zum Heben'
            BP.set_motor_position(BP.PORT_B,0)
            
            
            # Funktion zum Versenden einer Nachricht zum Client, z.B. "Name des Befehls", "IP Adresse", "Gruppenname"
            mqtt_pub("Erfolgreich gesenkt","192.168.0.99","Gruppe_5")
            time.sleep(0.02)                             # delay for 0.02 seconds (20ms) to reduce the Raspberry Pi CPU load.
            token = False
            msg = None
                
        if msg == 'Befehl zum Stoppen':                  # Stichwort, das via MQTT-fx eingegeben wird, z.B. 'Stop'                                      
            # Implementieren einer Funktion zum Stoppen des Hubwerks ohne die while-Schleife zu verlassen, nachdem durch Stichwort via MQTT getriggert wird
            position=BP.get_motor_encoder(BP.PORT_B)
            BP.set_motor_position(BP.PORT_B,position)
            # Schicken einer Stopp-Nachricht zum Client
            mqtt_pub("Durch den Benutzer gestoppt","192.168.0.99","Gruppe_5")
            BP.set_motor_limits(BP.PORT_B, 60, 300)      # set motor limits again if "reset sensors" was used
            time.sleep  (0.02)                           # delay for 0.02 seconds (20ms) to reduce the Raspberry Pi CPU load.
            token = False            
            msg = None

    except KeyboardInterrupt:
        BP.reset_all()                                       # reset all sensors




