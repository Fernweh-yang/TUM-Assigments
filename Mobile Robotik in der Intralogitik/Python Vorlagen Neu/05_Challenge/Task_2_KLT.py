#Import libraries
import time
import brickpi3
import functions
import follower


def main(BP):
    beladen = False
    barcode = ""
    while not barcode:
        barcode = functions.barcodescanner()
        print("Barcode:",barcode)
    stop = follower.line_follower(BP,400,7,0.01,[2])    
    while True:
        
        
        if stop == "Blue" and not beladen:
            print("Blau gefunden")
            functions.driveforward_mm(BP, functions.marker_offset)
            functions.turn(BP, 90)
            if barcode == functions.barcodescanner():
                print("Ladevorgang")
                functions.lift_klt(BP)
                beladen = True
                functions.turn(BP, -90)
                stop = follower.line_follower(BP,400,7,0.01,[5]) 
                continue
            print("Falscher KLT")
            functions.turn(BP, -90)
            stop = follower.line_follower(BP,400,7,0.01,[2]) 
        if stop == "Red":
            print("Fertig")
            break


