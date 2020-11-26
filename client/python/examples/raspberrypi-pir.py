# iot-facerecognition-client
# Example script for Raspberry Pi
# License: GPLv3
# Author: fuzun, 2020
# Address: github.com/fuzun/iot-facerecognition

# Description:
# This script allows Raspberry Pi to detect human presence through PIR sensor and check if the present human's face is recognized by the server.
# If a face is not detected, the red LED turns on. If a face detected but is unknown (no tag attached), the yellow LED turns on.
# And when a face is detected and is recognized, the green LED turns on.
# LED blink interval is 1.5 seconds.

# import FaceRecognition class from iot-facerecognition-client:
from iot_facerecognition_client import FaceRecognition

import RPi.GPIO as GPIO
import time
import sys
import threading
import os

# GPIO (BOARD) PINS, CONFIGURE AS YOU NEED
GREENLED_PIN  = 3
YELLOWRED_PIN = 5
REDLED_PIN    = 7
PIR_DATA_IN   = 10

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(GREENLED_PIN, GPIO.OUT)
GPIO.setup(YELLOWRED_PIN, GPIO.OUT)
GPIO.setup(REDLED_PIN, GPIO.OUT)
GPIO.setup(PIR_DATA_IN, GPIO.IN)

def turnoff():
    print("No face is detected!")
    GPIO.output(GREENLED_PIN, 0)
    GPIO.output(YELLOWRED_PIN, 0)
    GPIO.output(REDLED_PIN, 1)
timer = None

isStarted = False
def motionDetection(pin):
    global isStarted
    if GPIO.input(pin) and isStarted == False:
        print("Motion detected! Starting video capture...")
        faceRecognition.start()
        isStarted = True
    elif isStarted == True:
        print("Motion is not detected! Stopping video capture...")
        faceRecognition.stop()
        isStarted = False

def callback(tag):
    global timer
    if timer is not None:
        timer.cancel()
    if tag == "":
        print("Unknown face is detected!")
        GPIO.output(YELLOWRED_PIN, 1)
    else:
        print("Known face is detected! Tag:" + tag)
        GPIO.output(GREENLED_PIN, 1)
    GPIO.output(REDLED_PIN, 0)
    timer = threading.Timer(1.5, turnoff)
    timer.start()

if __name__ == "__main__":
    arglen = len(sys.argv)
    arglist = sys.argv
    
    ipaddr = "localhost"
    port = 50000
    
    if arglen == 3:
        ipaddr = arglist[1]
        port = arglist[2]
    
    faceRecognition = FaceRecognition("Raspberry-with-PIR-sensor", ipaddr, port, 1280, 720, callback, 0,
                                   0, True)
                                   
    print("Connecting to the server...")
    faceRecognition.connect()
    print("Connected.")
    GPIO.output(7, 1)
    GPIO.add_event_detect(PIR_DATA_IN, GPIO.BOTH, callback=motionDetection)
    motionDetection(PIR_DATA_IN)
    print("Started seeking for motion...")
    input('Press a key to terminate...')
    faceRecognition.stop()
    turnoff()
    GPIO.output(REDLED_PIN, 0)
    os._exit(os.EX_OK)
    