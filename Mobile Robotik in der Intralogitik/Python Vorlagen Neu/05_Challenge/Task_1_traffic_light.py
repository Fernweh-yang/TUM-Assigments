#Import the necessary libraries
import time
import brickpi3
import functions
import follower


def main(BP):
    greenVal = 0
    while greenVal < 0.4:
        greenVal = functions.traffic_light_detection()
        print("Anteil gruener Pixel: ", greenVal)
        time.sleep(0.5)
        


