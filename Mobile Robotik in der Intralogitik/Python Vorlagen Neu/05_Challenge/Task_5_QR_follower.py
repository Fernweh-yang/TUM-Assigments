#Import Libraries
import functions
import time
import brickpi3
import follower

def main(BP):
    barcode = 0
    #Erst Anfahren wenn QR erkannt
    while not barcode:
        barcode = functions.barcodescanner()
        time.sleep(0.5)
        print("Barcode erkannt:", barcode)
    follower.QR_follower(BP, 400, 7, 0.1)