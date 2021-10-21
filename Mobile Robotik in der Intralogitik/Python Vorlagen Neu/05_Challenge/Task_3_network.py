#Import libraries
import time
import brickpi3   
import follower   
import Dijkstra   #Contains the Dijkstra-Algorithm already implemented
import functions  

# 1) Write the “main” function

def main(BP):
    klt_node = 'i' #The node, where the KLT should be dropped
    klt = False
        
    #Determine the shortest route through Dijkstra
    shortestWay1 = Dijkstra.dijkstra('a',klt_node)
    shortestWay2 = Dijkstra.dijkstra(klt_node,'o')
    shortestWay2.pop(0)
    shortestWay = shortestWay1 + ["klt"] + shortestWay2
    print('My route: ' + str(shortestWay))
    shortestWay.pop(0)
    #Move the robot to the startnode
    ################
    stop = follower.line_follower(BP,400,7,0.01,[5])
    if stop == "Red":
        #functions.driveforward_mm(BP, functions.marker_offset)
        print("Startkoten erreicht")
    
    #Build a loop which leads the robot through the network by using the route calculated above by Dijkstra
    ################
    #Your code here#
    ################
    while len(shortestWay) != 0:
        
        find_new_path(BP, shortestWay, klt_node)
        print("Naechster Node")
        stop = follower.line_follower(BP,400,7,0.01,[5])


# 2) Write a function called “find_new_path” to let the robot find the correct new path at every node.
     #If the robot reaches the node where the KLT should be dropped, drop it there.

def find_new_path(BP, shortestWay, klt_node):
    #Drive forward to the center of the node and switch sensor mode 
    #BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_COLOR)
    functions.driveforward_mm(BP,210)
    klt_color = 3   #3 = "Green"
    nextColor = functions.find_pathcolor(shortestWay.pop(0))
    print("nextColor: ", nextColor)
    

    #Build a loop to find the new path and drop the klt first, if required
    ################
    #Your code here#
    ################
    while True:
        functions.turn(BP, -4)
        #time.sleep(0.05)
        color = colorCheck(BP)
        blackVal = BP.get_sensor(BP.PORT_3)
        print("Schwarzwert:", blackVal)
        if nextColor == 3: #Bis zum gruenen Drehen
            if color == nextColor and blackVal < 30:
                print("Stelle Klt ab...")
                functions.drop_klt(BP)
                nextColor = functions.find_pathcolor(shortestWay.pop(0))
                continue
        else:
            if color == nextColor and blackVal < 30: 
                print("Weg gefunden")
                break
    
    #Reset sensor and drive forward to follow the correct outgoing path
    #BP.set_sensor_type(BP.PORT_3, BP.SENSOR_TYPE.EV3_COLOR_REFLECTED)
    functions.driveforward_mm(BP,functions.marker_offset)
    
def colorCheck(BP):
    color_check = []
    try:
        while True:#Vergleich von nacheinander erkannten Farben
            for i in range(1,10):
                valueA = BP.get_sensor(BP.PORT_4)
                color_check.append(valueA)
                time.sleep(0.005)
            
            return most_common(color_check)
            
    except brickpi3.SensorError as error:
        print(error)
def most_common(lst):
    return max(set(lst), key=lst.count)