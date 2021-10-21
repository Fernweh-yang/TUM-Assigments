#Import libraries
import time
import functions
import follower

def main(BP, target_form):
    beladen = True
    stop = follower.line_follower(BP,400,7,0.01,[2])
    while True:
        
        if stop == "Blue" and beladen:
            print("Blau gefunden")
            functions.driveforward_mm(BP, functions.marker_offset)
            functions.turn(BP, 90)
            if functions.shape_detection(target_form):
                print("Ladevorgang")
                functions.drop_klt(BP)
                beladen = False
                functions.turn(BP, -90)
                stop = follower.line_follower(BP,250,7,0.01,[5])
                continue
            print("Falscher Abladeort")
            functions.turn(BP, -90)
            stop = follower.line_follower(BP,400,7,0.01,[2])
        if stop == "Red":
            print("Fertig")
            break
         
