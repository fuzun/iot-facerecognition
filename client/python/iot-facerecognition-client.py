# iot-facerecognition-client
# License: GPLv3
# Author: fuzun, 2020
# Address: github.com/fuzun/iot-facerecognition

import cv2
#import wsaccel
import websocket
import time
import threading
import sys

class FaceRecognition:
    def __init__(self, clientname, ipaddress, port, capturewidth, captureheight, callbackfunc=None,
                 targetwidth=0, targetheight=0, lessbandwidth=False, enablewstrace=False, guienabled=False,
                 guiwidth=0, guiheight=0):
        self.clientname = clientname
        self.ipaddress = ipaddress
        self.port = port
        self.callback = callbackfunc
        self.capturewidth = capturewidth
        self.captureheight = captureheight
        self.targetwidth = targetwidth
        self.targetheight = targetheight
        self.lessBandwidth = lessbandwidth
        self.enablewstrace = enablewstrace
        self.callbackfunc = callbackfunc
        self.guienabled = guienabled
        self.guiwidth = guiwidth
        self.guiheight = guiheight

        if self.guiwidth > 0 and self.guiheight > 0:
            self.nFactorW = float(guiwidth) / float(targetwidth)
            self.nFactorH = float(guiheight) / float(targetheight)
        else:
            self.nFactorH = 1
            self.nFactorW = 1

        self.time = 0
        self.cap = None
        self.ws = None
        self.msg = None
        self.cbackcalled = None

        self.stop = False
        self.thread1 = threading.Thread(target=self.mainloop)
        self.thread2 = threading.Thread(target=self.receiveloop)

    def connect(self):
        if self.enablewstrace:
            websocket.enableTrace(True)
        self.ws = websocket.create_connection("wss://" + str(self.ipaddress) + ":" + str(self.port))
        self.ws.send("0:" + self.clientname)

    def stop(self):
        self.stop = True
        self.thread1.join()
        self.thread2.join()
        self.stop = False

    def start(self):
        self.cap = cv2.VideoCapture(0)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.capturewidth)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.captureheight)
        self.thread1.start()
        self.thread2.start()

    def mainloop(self):
        while not self.stop:
            ret, frame = self.cap.read()
            frame2 = frame

            if self.targetwidth > 0 and self.targetheight > 0:
                frame = cv2.resize(frame, (self.targetwidth, self.targetheight), interpolation=cv2.INTER_AREA)

            if self.guiwidth > 0 and self.guiheight > 0:
                frame2 = cv2.resize(frame2, (self.guiwidth, self.guiheight))

            if self.lessBandwidth is False:
                ret2, encoded = cv2.imencode('.jpg', frame)
            else:
                ret2, encoded = cv2.imencode('.webp', frame)

            self.ws.send_binary(encoded.tobytes())

            if self.guienabled:
                if time.time() - self.time < 3 and self.msg is not None:
                    for msg in self.msg.split(':'):
                        pmsg = msg.split(',')
                        if self.callbackfunc is not None and self.cbackcalled is False and str(pmsg[4]) != '':
                            self.callbackfunc(str(pmsg[4]))
                            self.cbackcalled = True
                        x0 = int(pmsg[0]) * self.nFactorW
                        x1 = int(pmsg[2]) * self.nFactorW
                        y0 = int(pmsg[1]) * self.nFactorH
                        y1 = int(pmsg[3]) * self.nFactorH
                        frame = cv2.rectangle(frame2, (int(x0), int(y0)), (int(x1), int(y1)),
                                              (int(128), int(0), int(0)), 3)
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        cv2.putText(frame2, str(pmsg[4]), (int(x0), int(y1 + 25)), font, 1.5, (0, 0, 200), 2,
                                    cv2.LINE_AA)
                cv2.imshow('facerecognition', frame2)
                cv2.waitKey(10)

    def receiveloop(self):
        while not self.stop:
            self.msg = self.ws.recv()
            self.cbackcalled = False
            self.time = time.time()

    def sendmessage(self, message):
        self.ws.send("1:" + message)

if __name__ == "__main__":
    arglen = len(sys.argv)
    arglist = str(sys.argv)

    clientname = "default"
    ipaddr = "localhost"
    port = 50000
    capwidth = 1920
    capheight = 1080
    #callback = None
    #targetwidth = 1280
    #targetheight = 720
    #lessbandwidth = False
    #enablewstrace = False
    #guienabled = True

    if arglen == 6:
        clientname = arglist[1]
        ipaddr = arglist[2]
        port = int(arglist[3])
        capwidth = int(arglist[4])
        capheight = int(arglist[5])

    frecognition = FaceRecognition(clientname, ipaddr, port, capwidth, capheight, None, 1280, 720, False, False, True, 1920, 1080)
    frecognition.connect()
    frecognition.start()
