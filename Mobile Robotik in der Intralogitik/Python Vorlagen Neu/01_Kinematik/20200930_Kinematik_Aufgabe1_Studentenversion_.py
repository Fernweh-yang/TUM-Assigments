from __future__ import print_function
from __future__ import division           

import time as t
import brickpi3
import math

inf = math.inf #Unendlich definieren



########################################
#BrickPi und dessen Anschlüsse definieren
#----------------------------------------
# BrickPi instanziieren
BP = brickpi3.BrickPi3()

#Motoranschlüsse definieren
motor_l = BP.PORT_D 
motor_r = BP.PORT_A

#Motorlimit NICHT ENTFERNEN!!!
#Motoren begrenzt auf 90% der Nennleistung
BP.set_motor_limits(motor_l, 90, 0)
BP.set_motor_limits(motor_r, 90, 0)


#Sensoranschlüsse definieren
sensor_distance = BP.PORT_1 #Abstandssensor an Anschluss 1
sensor_gyro = BP.PORT_2     #Gyrosensor an Anschluss 2

BP.set_sensor_type(sensor_distance, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM) #Abstand an Anschluss 1
BP.set_sensor_type(sensor_gyro, BP.SENSOR_TYPE.EV3_GYRO_ABS) #Gyro an Anschluss 2
########################################

#Weltkoordinaten
new_position = [0,0,0,0] #x in m, y in m, Winkel in °, Winkel aus Gyro in °
coordinates = [0,0,0] #x in m, y in m, Winkel in °

########################################
#Fahrzeuggeometrie
# Eingabe der Fahrzeugparameter und Berechnung weiterer Werte
# z_motor [-]:  Anzahl Zähne am Zahnrad auf der Motorseite
# z_wheel [-]:  Anzahl Zähne am Zahnrad auf der Radseite
# d_wheel [mm]: Raddurchmesser
# ratio [-]: Getriebeübersetzung; Berechnung aus den Zähnezahlen
# track_inside [mm]: Abstand zwischen der Innenseite der beiden Räder
# track_outside [mm]: Abstand zwischen der Außenseite der beiden Räder
# track [m]: Spurbreite (Abstand der Mittenpunkte der beiden Räder) - Angabe in Meter!!



#HIER MÜSSEN ERGÄNZUNGEN VORGENOMMEN WERDEN
z_motor = 8
z_wheel = 24
d_wheel = 
track_inside = 
track_outside =

#Geometrieberechnung
ratio = z_wheel / z_motor #Getriebeübersetzung
track =                    #Achtung: Angabe in der Einheit m!
#ENDE DER ERGÄNZUNGEN

########################################


########################################
# Fahrfunktion
########################################
# Beschreibung
#---------------------------------------
# Ausführen von Fahrbefehlen je nach Eingabe:
# Geradeausfahren: v = Robotergeschwindigkeit
#                  radius = math.inf
#                  omega = 0
# Kurvenfahren:    v = Robotergeschwindigkeit
#                  radius = Kurvenradius (> 0 --> Linkskurve; < 0 Rechtskurve)
#                  omega = 0
# Drehen auf der Stelle: v = 0
#                        radius = 0
#                        omega = Drehgeschwindigkeit des Roboters
########################################
# Eingangsparameter
#---------------------------------------
# v [m/s]: Geschwindigkeit
# radius [m]: Kurvenradius
# omega [°/s]: Winkelgeschwindigkeit für Drehung auf der Stelle
# last_call [s]: Zeipunkt des letzten Methodenaufrufs
# last_position [m,m,°]: Position beim letzten Funktionsaufruf im globalen Koordinatensystem
# last_speed [m/s]: Geschwindigkeit beim letzten Funktionsaufruf
# last_radius [m]: Kurvenradius beim letzten Funktionsaufruf
# last_omega [°/s]: Winkelsgeschwindigkeit beim letzten Funktionsaufruf
########################################
# Parameter innerhalb der Funktion
#---------------------------------------
# current_call [s]: aktueller Zeitpunkt
# delta_t [s]: vergangene Zeit seit dem letzten Aufruf
# last_v_r, last_v_l [m/s]: Geschwindigkeit am rechten/linken Rad beim letzten Funktionsaufruf
# new_position[x,y,theta,theta(aus gyro)] [m,m,°,°] : Vektor mit der aktuellen Position des Roboters
# omega_z [rad/s] : Rotationsgeschwindigkeit des Roboters um die eigene z-Achse
# omega_r, omega_l [rad/s] : Drehgeschwindigkeit des rechten/linken Rades
# dps_r, dps_l [°/s] : Drehgeschwindigkeit des rechten/linken Motors
########################################
# Funktionsausführung
def Fahren(v, radius, omega, last_call, last_position, last_speed, last_radius, last_omega): #Einheiten: m/s; m; °/s; s; [m,m,°]; m/s; m; °/s
     
    
    #Odometriedaten seit dem letzten Aufruf berechnen
    current_call = t.perf_counter()
	delta_t = current_call - last_call

    if math.isinf(radius): # Geradeausfahren, d. h. radius = unendlich
        last_omega_z = 0
        last_v_r = last_speed 
        last_v_l = last_speed     
    elif last_radius != 0: #Kurve fahren um den Momentanpol
        if last_radius==0: 
            last_omega_z=0
        else:
            last_omega_z = (last_speed / last_radius)*180/math.pi #Drehgeschwindigkeit berechnen
        last_v_r = last_speed + (track/2)*last_omega_z
        last_v_l = last_speed - (track/2)*last_omega_z
    elif last_speed==0 and last_radius==0: #Drehen auf der Stelle
        last_omega_z = last_omega
        last_v_r = (track/2)*last_omega_z
        last_v_l = - (track/2)*last_omega_z

#HIER MÜSSEN ERGÄNZUNGEN VORGENOMMEN WERDEN    
    #Odometrieberechnung
    #new_position = [x-Position, y-Position, Orientierung (z-Winkel)], Orientierung (z-Winkel aus Gyro)]
	new_position[0]=
	new_position[1]=  
	new_position[2]=


#ENDE DER ERGÄNZUNGEN
	new_position[3] = -1*BP.get_sensor(BP.PORT_2) #Winkel aus Gyro in °
	

    #Koordinaten werden in Aufgabe 1 noch nicht benötigt
	coordinates[0] = 0
	coordinates[1] = 0
	coordinates[2] = 0
        
    
    #neue Geschwindigkeit setzen
    if v==0 and radius==0: # auf der Stelle drehen
        omega_z = omega * math.pi / 180 #Umrechnung von °/s in rad/s
        omega_r = ((track / (d_wheel/1000))*omega_z) #Ergebnis in rad/s
        omega_l = -((track / (d_wheel/1000))*omega_z) #Ergebnis in rad/s        
        dps_r = (ratio * omega_r * 360) / (2*math.pi) #Umrechnung von rad/s in °/s
        dps_l = (ratio * omega_l * 360) / (2*math.pi) #Umrechnung von rad/s in °/s
    elif math.isinf(radius) and omega == 0 and v != 0: #Geradeaus fahren
        omega_r = ((2 / (d_wheel/1000))*v)  #Ergebnis in rad/s
        omega_l = ((2 / (d_wheel/1000))*v)  #Ergebnis in rad/s        
        dps_r = (ratio * omega_r * 360) / (2*math.pi) #Umrechnung von rad/s in °/s
        dps_l = (ratio * omega_l * 360) / (2*math.pi) #Umrechnung von rad/s in °/s
        
    elif radius != 0 and v != 0:   #Kurve fahren, negatives Omega: Rechtskurve, positives Omega: Linkskurve
        omega_z = v / radius #Drehgeschwindigkeit berechnen
        omega_r = ((2 / (d_wheel/1000))*v) + ((track / (d_wheel/1000))*omega_z) #Ergebnis in rad/s
        omega_l = ((2 / (d_wheel/1000))*v) - ((track / (d_wheel/1000))*omega_z) #Ergebnis in rad/s        
        dps_r = (ratio * omega_r * 360) / (2*math.pi) #Umrechnung von rad/s in °/s
        dps_l = (ratio * omega_l * 360) / (2*math.pi) #Umrechnung von rad/s in °/s
    
    # Geschwindigkeiten der Motoren setzen
    BP.set_motor_dps(motor_r, dps_r)
    BP.set_motor_dps(motor_l, dps_l)
    
    # Rückgabe der neu berechneten Position
    return new_position
########################################    


########################################
# Stoppfunktion
########################################
# Beschreibung
#---------------------------------------
# setzt die Drehzahl der Motoren auf 0
########################################
def stop():
    BP.set_motor_power(motor_r, 0)
    BP.set_motor_power(motor_l, 0)
########################################    



   
########################################
# Hauptfunktion
########################################
# Beschreibung
#---------------------------------------
# Die Funktion lässt den Roboter einfache Fahrbefehle ausführen.
# Über die Variable "Fahrbefehl" wird eine vordefinierte Aufgabe erledigt
#   - "Geradeausfahrt" : 2 m geradeaus mit einer Geschwindigkeit von 0,1 m/s
#   - "Kurvenfahrt" : Kreis mit einem Radius von 0,5 m und mit einer Geschwindigkeit von 0,1 m/s
#   - "Drehung" : Drehung auf der Stelle um 360° mit einer Geschwindigkeit von 15 °/s
########################################
# Parameter innerhalb der Funktion
#---------------------------------------
# last_position [m,m,°]: Position beim letzten Funktionsaufruf im globalen Koordinatensystem
# last_call [s]: Zeipunkt des letzten Methodenaufrufs
# last_speed [m/s]: Geschwindigkeit beim letzten Funktionsaufruf
# last_radius [m]: Kurvenradius beim letzten Funktionsaufruf
# last_omega [°/s]: Winkelgeschwindigkeit beim letzten Funktionsaufruf
# new_speed  [m/s] : neu gesetzte Geschwindigkeit
# new_radius [m] : neu gesetzter Kurvenradius
# new_omega [°/s] : neu gesetzte Winkelgeschwindigkeit
# Fahrbefehl [-] : Auswahl der Fahraufgabe
# control_value [-] : normierter Fortschritt der Bewegung
########################################
# Funktionsausführung
try:
    t.sleep(5) 
    
    # Koordinaten zurücksetzen
    last_position = [0, 0, 0, 0] # x, y, theta, theta(gyro)
    last_call = t.perf_counter()
    last_speed = 0
    last_radius = 0
    last_omega = 0
    control_value = 0
    
    print(last_position)
    print(coordinates)
    
#HIER KANN DURCH AUSKOMMENTIEREN FESTGELEGT WERDEN, WELCHE FAHRFUNKTION ANGESPROCHEN WIRD.
    # Eingabe der Fahrfunktion
    Fahrbefehl = "Geradeausfahrt"
    #Fahrbefehl = "Kurvenfahrt"
    #Fahrbefehl = "Drehung"
               
#HIER WERDEN DURCH WERTVORGABE DIE PARAMETER GESCHWINDIGKEIT, RADIUS UND OMEGA EINGEGEBEN      
    # Auswahl der Fahrbefehle
    if Fahrbefehl == "Geradeausfahrt":
        new_speed =  #m/s; v < 0.15 m/s; v<0 ist Rückwärtsfahrt
        new_radius = math.inf #m, bei Geradeausfahrt: math.inf; Radius > 0: Linkskurve; Radius < 0: Rechtskurve
        new_omega = 0 #°/s wird nur für Drehung auf der Stelle benötigt
    elif Fahrbefehl == "Kurvenfahrt":
        new_speed =  #m/s; v < 0.15 m/s; v<0 ist Rückwärtsfahrt
        new_radius =  #m, bei Geradeausfahrt: math.inf; Radius > 0: Linkskurve; Radius < 0: Rechtskurve
        new_omega = 0 #°/s wird nur für Drehung auf der Stelle benötigt
    elif Fahrbefehl == "Drehung":
        new_speed = 0 #m/s; v < 0.15 m/s; v<0 ist Rückwärtsfahrt
        new_radius = 0 #m, bei Geradeausfahrt: math.inf; Radius > 0: Linkskurve; Radius < 0: Rechtskurve
        new_omega =  #°/s wird nur für Drehung auf der Stelle benötigt
    else:
        print(error)

    while True:
#HIER MÜSSEN ERGÄNZUNGEN VORGENOMMEN WERDEN, damit der Roboter fährt    
        while control_value < ???: # solange die Bewegung noch nicht abgeschlossen ist, d.h. solange Abbruchkriterium nicht zutrifft.
#ENDE DER ERGÄNZUNGEN            
            t.sleep(0.1) # bei kleineren Werten (0,001) gab es Probleme mit der Aus- und Eingabe.
            try:
                distance = BP.get_sensor(BP.PORT_1) # print the distance sensor values
            except brickpi3.SensorError as error:
                print(error)     
            if distance < 10:
                stop()
                break
                     
            #  Beim ersten Aufruf für delta_t sorgen
            if control_value == 0:
                last_call = t.perf_counter()
            
            # Aufruf der Fahrfunktion     
            last_position = Fahren(new_speed, new_radius, new_omega, last_call, last_position, last_speed, last_radius, last_omega)      
#HIER MÜSSEN ERGÄNZUNGEN VORGENOMMEN WERDEN, damit der Roboter fährt            
            # Kontrollwert je nach Bewegungstyp setzen
            if Fahrbefehl == "Geradeausfahrt":
				control_value = 
            elif Fahrbefehl == "Kurvenfahrt":
				control_value = 
            elif Fahrbefehl == "Drehung":
				control_value = 
#ENDE DER ERGÄNZUNGEN            
            else:
                print(error)
            # aktuellen Werte als vergangene Werte für den nächsten Aufruf merken
            last_speed = new_speed
            last_radius = new_radius
            last_omega = new_omega
            last_call = t.perf_counter() # aktuellen Zeitpunkt für den nächsten Aufruf merken
            
        stop()
        print(new_position)
        print(coordinates)
        
        break
        BP.reset_all()
    
except KeyboardInterrupt:
    BP.reset_all()

