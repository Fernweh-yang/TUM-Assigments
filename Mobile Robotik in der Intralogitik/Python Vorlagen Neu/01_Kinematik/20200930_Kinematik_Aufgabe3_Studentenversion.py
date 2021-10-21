from __future__ import print_function
from __future__ import division           

import time as t
import brickpi3
import math
import numpy as np

inf = math.inf #Unendlich definieren
#Test for infinity: x = inf; math.isinf(x)


########################################
#BrickPi und dessen Anschlüsse definieren
#----------------------------------------
# BrickPi instanziieren
BP = brickpi3.BrickPi3()

#Motoranschlüsse definieren
motor_l = BP.PORT_D 
motor_r = BP.PORT_A

# Sensoranschlüsse definieren
sensor_distance = BP.PORT_1 #Abstandssensor an Anschluss 1
sensor_gyro = BP.PORT_2     #Gyrosensor an Anschluss 2

#Sensoranschlüsse definieren
BP.set_sensor_type(BP.PORT_1, BP.SENSOR_TYPE.EV3_ULTRASONIC_CM) #Abstand an Anschluss 1
BP.set_sensor_type(BP.PORT_2, BP.SENSOR_TYPE.EV3_GYRO_ABS) #Gyro an Anschluss 2

#Motorlimit NICHT ENTFERNEN!!!
#Motoren begrenzt auf 90% der Nennleistung
BP.set_motor_limits(motor_l, 90, 0)
BP.set_motor_limits(motor_r, 90, 0)
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
# track_inside [mm]:  Abstand der Innenflächen der Räder
# track_outside [mm]: Abstand der Außenflächen der Räder
# track_compensation [-]: Korrekturfaktor für den nicht mittigen Radaufstand
# ratio [-]: Getriebeübersetzung; Berechnung aus den Zähnezahlen
# track [mm]: Spurbreite; Berechnung aus den Radabständen mit dem Korrekturfaktor

z_motor = 8
z_wheel = 24
d_wheel = 72 #Raddurchmesser (in mm)
track_inside = 132 #innerer Radabstand (in mm)
track_outside = 204 #äußerer Radabstand (in mm)
track_compensation = 0.94 #Kompensationsfunktion, weil wir den Radabstand überschätzen (Rad steht nicht genau mittig auf)

#Geometrieberechnung
ratio = z_wheel / z_motor #Getriebeübersetzung
track = ((track_inside + (track_outside - track_inside)/2)/1000)*track_compensation  #Spurbreite (in m)
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
# last_call [s]: Zeitpunkt des letzten Methodenaufrufs
# last_position [m,m,°]: Position beim letzten Funktionsaufruf im globalen Koordinatensystem
# last_speed [m/s]: Geschwindigkeit beim letzten Funktionsaufruf
# last_radius [m]: Kurvenradius beim letzten Funktionsaufruf
# last_omega [°/s]: Winkelgeschwindigkeit beim letzten Funktionsaufruf
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
def Fahren(v, radius, omega, last_call, last_position, last_speed, last_radius, last_omega): #Einheiten: m/s; m; °/s; s; [m,m,°]; m/s; m, °/s
     
    
    #Odometriedaten seit dem letzten Aufruf berechnen
    current_call = t.perf_counter()
    delta_t = current_call - last_call
    #print(delta_t)
    if math.isinf(last_radius): # Geradeausfahren, d. h. radius = unendlich
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
    
     
    #Odometrieberechnung 
    #new_position = [x-Position, y-Position, Orientierung (z-Winkel)], Orientierung (z-Winkel aus Gyro)]
    new_position[0] = last_position[0] + last_speed*delta_t*math.cos(last_position[2] + ((((last_v_r - last_v_l)/track) * delta_t) / 2)) #x-Position
    new_position[1] = last_position[1] + last_speed*delta_t*math.sin(last_position[2] + ((((last_v_r - last_v_l)/track) * delta_t) / 2)) #y-Position
    new_position[2] = last_position[2] + (((last_v_r - last_v_l)/track) * delta_t) #Orientierung in °
    new_position[3] = -1*BP.get_sensor(sensor_gyro) #Winkel aus Gyro in °
    #print(new_position)
    coordinates[0] = coordinates[0] + last_speed*delta_t*math.cos((coordinates[2]*math.pi) / 180)# + ((((last_v_r - last_v_l)/track) * delta_t) / 2)) #x-Position
    coordinates[1] = coordinates[1] + last_speed*delta_t*math.sin((coordinates[2]*math.pi) / 180)# + ((((last_v_r - last_v_l)/track) * delta_t) / 2)) #y-Position
    coordinates[2] = coordinates[2] + (((last_v_r - last_v_l)/track) * delta_t) #Orientierung in °
    
    #neue Geschwindigkeiten der Motoren berechnen
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
# Die Funktion lässt den Roboter beliebig viele Wegpunkte abfahren, die in einer Matrix eingegeben werden.
# Es wird immer geradlinig von Punkt zu Punkt gefahren und dort gedreht.
########################################
# Parameter innerhalb der Funktion
#---------------------------------------
# point_start = [x,y] [m,m] : Startpunkt des Roboters. Wird auf [0,0] gesetzt
# points = [[x1,y1],[x2,y2],..] [[m,m],[m,m],..] : Matrix mit allen Wegpunkten
# number_of_points : Größe der "points"-Matrix
# Distances :  Liste mit allen berechneten Entfernungen zwischen den Wegpunkten
# Angles : Liste mit allen Winkeln zwischen den Verbindungen der Wegpunkte (entspr. den Drehwinkeln an den Punkten)
# position_end : Zielposition für den Fahrbefehl (Strecke oder Winkel)
# determinante : Wert der Determinante für die Bestimmung der Drehrichtung
# direction1, direction2 : (Richtungs-)Vektoren des letzten und nächsten Abschnitts
# dot : Vektorprodukt der beiden aktuellen Richtungen
# length1, length2 : Längen der beiden aktuellen Vektoren
# angle : Winkel zwischen den beiden aktuellen Vektoren
# new_speed  [m/s] : neu gesetzte Geschwindigkeit
# new_radius [m] : neu gesetzter Kurvenradius
# new_omega [°/s] : neu gesetzte Winkelgeschwindigkeit
# i, k, n, m : Laufvariablen
########################################
# Funktionsausführung
try:
    t.sleep(5) # Wartezeit, bis der Sensor bereit ist

    ########################################
    #Definition der Wegpunkte in Matrix, x-Koordinate, y-Koordinate
    point_start = np.array([0,0])
    points = 

    
    ########################################
    #Berechnung der Entfernungen und Drehwinkel
    number_of_points = points.shape
    Distances = np.zeros(number_of_points[0])
    Angles = np.zeros(number_of_points[0])
    position_end = 0
    determinante = 0
    i = 0
    for i in range(0, number_of_points[0]):  
        if i==0:
            direction1=np.array([1, 0]) # Orientierung des Roboters zu Beginn
            direction2=points[i,:]         # Ortsvektor des ersten Punktes
        else:
            direction2=points[i,:]-points[i-1,:] # Verbindungsvektor vom aktuellen zum nächsten Punkt
		
		dot=
		length1= 
		length2= 
        if length1 == 0 or length2 == 0:
            angle=0 
        else:

            angle = 
            # Prüfen ob Drehung nach links oder rechts
            determinante = direction1[0]*direction2[1] - direction1[1]*direction2[0]
            if determinante < 0:
                angle = -1*angle
        Distances[i]=length2 # Aufnahme der Entfernung in die Liste der Fahrbefehle
        direction1=direction2 # merken der Richtung für den nächsten Schritt
        Angles[i]=angle # Aufnahme des Winkels in die Liste der Drehbefehle         
        i=i+1
    ColumnsRows=Distances.shape
    ########################################

    i = 0
    k = 0
    n = 0
    m = 0     
    last_position = [0, 0, 0, 0] # x, y, theta, theta(gyro)
    print(last_position)
    print(coordinates)
    
    while True:
        while i < 2*ColumnsRows[0]: #alle Punkte der Tabelle abfahren
            compare = 0
            k =  i + 1
            if k%2 != 0:
                # Koordinaten zurücksetzen
                last_position = [0, 0, 0, 0] # x, y, theta, theta(gyro)
                last_call = t.perf_counter()
                last_speed = 0
                last_radius = 0
                last_omega = 0
                position_end = Angles[m]*180/math.pi
                new_speed = 0 #m/s; v < 0.15 m/s; v<0 ist Rückwärtsfahrt
                new_radius = 0 #m, bei Geradeausfahrt: math.inf; Radius > 0: Linkskurve; Radius < 0: Rechtskurve
                if position_end < 0:
                    new_omega = -10 #°/s wird nur für Drehung auf er Stelle benötigt
                else:
                    new_omega = 10    
                m = m + 1
                winkel=1
            else:
                # Koordinaten zurücksetzen
                last_position = [0, 0, 0, 0] # x, y, theta, theta(gyro)
                last_call = t.perf_counter()
                last_speed = 0
                last_radius = 0
                last_omega = 0
                position_end = Distances[n]
                new_speed = 0.1 #m/s; v < 0.15 m/s; v<0 ist Rückwärtsfahrt
                new_radius = math.inf #m, bei Geradeausfahrt: math.inf; Radius > 0: Linkskurve; Radius < 0: Rechtskurve
                new_omega = 0 #°/s wird nur für Drehung auf er Stelle benötigt
                n = n + 1 
                winkel=0
            i = i + 1
            while abs(compare) < abs(position_end): # solange der Teilabschnitt nicht abgeschlossen ist
                t.sleep(0.1) # bei kleineren Werten (0,001) gab es Probleme mit der Aus- und Eingabe.
                try:

                    distance = BP.get_sensor(sensor_distance) 
                except brickpi3.SensorError as error:
                    print(error) 
                if distance < 10:    
                    stop()
                    break
                else:
                    #  Beim ersten Aufruf für delta_t sorgen
                    if compare == 0:
                        last_call = t.perf_counter()
                        
                    # Aufruf der Fahrfunktion    
                    last_position = Fahren(new_speed, new_radius, new_omega, last_call, last_position, last_speed, last_radius, last_omega) # Fahrfunktion aufrufen             
                    
                    # Kontrollwert je nach Bewegungstyp setzen
                    if winkel !=0:
						compare=last_position[2]
                    else:
						compare=math.sqrt(last_position[0]*last_position[0]+last_position[1]*last_position[1])
                    last_speed = new_speed # aktuellen Werte als vergangene Werte für den nächsten Aufruf merken
                    last_radius = new_radius
                    last_omega = new_omega
                    last_call = t.perf_counter() # aktuellen Zeitpunkt für den nächsten Aufruf merken
            stop()
            print(new_position)
            print(coordinates)
        stop()
        BP.reset_all()
except KeyboardInterrupt:
    BP.reset_all()

