# iot-facerecognition-client
# License: GPLv3
# Author: fuzun, 2020
# Address: github.com/fuzun/iot-facerecognition

import ssl
import cv2
import websocket
import screeninfo
import time
import threading
import sys
import json
from enum import IntEnum

class FaceRecognition:

    class Command(IntEnum): # should be in sync with Server's Client::Command enum class
        CHANGE_NAME = 1,
        MESSAGE = 2,
        MESSAGE_TAG_FACE = 3,
        MESSAGE_TAG_OBJECT = 4,
        SETTING_OBJDETECTIONENABLED = 5,
        SETTING_LABELCOUNT = 6
            
    def __init__(self, clientname, ipaddress, port, capturewidth, captureheight, callbackfunc=None,
                 targetwidth=0, targetheight=0, guienabled=False,
                 enablewstrace=False, allowselfsign=True):

        self.clientname = clientname
        self.ipaddress = ipaddress
        self.port = port
        self.callback = callbackfunc
        self.capturewidth = capturewidth
        self.captureheight = captureheight
        self.targetwidth = targetwidth
        self.targetheight = targetheight
        if self.targetwidth == 0 or self.targetheight == 0:
            self.targetwidth = self.capturewidth
            self.targetheight = self.captureheight
        self.enablewstrace = enablewstrace
        self.callbackfunc = callbackfunc
        self.guienabled = guienabled
        self.allowselfsign = allowselfsign

        self.time = [0, 0]
        self.cap = None
        self.ws = None
        self.msg = [None, None]
        self.cbackcalled = None

        self._stop = False
        self.thread1 = threading.Thread(target=self.mainloop)
        self.thread2 = threading.Thread(target=self.receiveloop)

    def sendCommand(self, cmd, ctx):
        self.ws.send(chr(int(cmd)) + ":" + str(ctx))

    def connect(self):
        if self.enablewstrace:
            websocket.enableTrace(True)
        if self.allowselfsign:
            self.ws = websocket.WebSocket(sslopt={"cert_reqs": ssl.CERT_NONE})
        else:
            self.ws = websocket.WebSocket()
        self.ws.connect("wss://" + str(self.ipaddress) + ":" + str(self.port))
        self.sendCommand(FaceRecognition.Command.CHANGE_NAME, self.clientname)

    def stop(self):
        self._stop = True
        self.cap.release()

    def start(self):
        self.cap = cv2.VideoCapture(0)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.capturewidth)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.captureheight)
        if self._stop == True:
            self._stop = False
        else:
            self.thread1.start()
            self.thread2.start()

    def mainloop(self):
        if self.guienabled:
            cv2.namedWindow('facerecognition', cv2.WND_PROP_FULLSCREEN)
            cv2.setWindowProperty('facerecognition', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
            screen = screeninfo.get_monitors()[0]
            self.guiwidth, self.guiheight = screen.width, screen.height
            self.nFactorW = float(self.guiwidth) / float(self.targetwidth)
            self.nFactorH = float(self.guiheight) / float(self.targetheight)

        while True:
            if not self._stop:
                ret, frame = self.cap.read()
                frame2 = frame

                if self.targetwidth > 0 and self.targetheight > 0 and \
                        self.targetheight != self.captureheight and self.targetwidth != self.capturewidth:
                    frame = cv2.resize(frame, (self.targetwidth, self.targetheight), interpolation=cv2.INTER_AREA)

                if self.guienabled and abs(self.guiwidth - self.capturewidth) > 5 \
                        and abs(self.guiheight - self.captureheight) > 5:
                    frame2 = cv2.resize(frame2, (self.guiwidth, self.guiheight))

                ret2, encoded = cv2.imencode('.jpg', frame)

                self.ws.send_binary(encoded.tobytes())

                if self.guienabled:
                    if time.time() - self.time[0] < 0.75 and self.msg[0] is not None:
                        for msg in self.msg[0]:
                            tag = str(msg["tag"])
                            x = int(msg["x"])
                            y = int(msg["y"])
                            width = int(msg["width"])
                            height = int(msg["height"])
                            if self.callbackfunc is not None and self.cbackcalled is False:
                                self.callbackfunc(tag)
                                self.cbackcalled = True
                            x0 = x * self.nFactorW
                            x1 = (x + width) * self.nFactorW
                            y0 = y * self.nFactorH
                            y1 = (y + height) * self.nFactorH
                            cv2.rectangle(frame2, (int(x0), int(y0)), (int(x1), int(y1)),
                                                  (int(128), int(0), int(0)), 3)
                            font = cv2.FONT_HERSHEY_SIMPLEX
                            cv2.putText(frame2, tag, (int(x0), int(y1 + 25)), font, 1.5, (0, 0, 200), 2,
                                        cv2.LINE_AA)
                    if time.time() - self.time[1] < 0.75 and self.msg[1] is not None:
                        counter = 0
                        for msg in self.msg[1]:
                            font = cv2.FONT_HERSHEY_SIMPLEX
                            cv2.putText(frame2, "{}: {}".format(str(msg["prediction"]), str(msg["label"])), (int(20), int(20 + counter * 20)), font, 0.5, (0, 0, 200), 1,
                                        cv2.LINE_AA)
                            counter = counter + 1
                    cv2.imshow('facerecognition', frame2)
                    cv2.waitKey(5)
            else:
                time.sleep(0.1)

    def receiveloop(self):
        while True:
            if not self._stop:
                receive = self.ws.recv()

                jdoc = json.loads(receive)
                cmd = jdoc["command"]
                hasCtx = "context" in jdoc
                ctx = None
                if (hasCtx):
                    ctx = jdoc["context"]

                if cmd is int(FaceRecognition.Command.MESSAGE_TAG_FACE):
                    self.msg[0] = ctx
                    self.time[0] = time.time()
                elif cmd is int(FaceRecognition.Command.MESSAGE_TAG_OBJECT):
                    self.msg[1] = ctx
                    self.time[1] = time.time()

                self.cbackcalled = False
            else:
                time.sleep(0.1)

    def sendmessage(self, message):
        self.sendCommand(FaceRecognition.Command.MESSAGE, message)


def main():
    arglen = len(sys.argv)
    arglist = sys.argv

    clientname = "default"
    ipaddr = "localhost"
    port = 50000
    capwidth = 1920
    capheight = 1080
    targetwidth = 1280
    targetheight = 720

    if arglen >= 2:
        clientname = arglist[1]
    if arglen >= 4:
        ipaddr = arglist[2]
        port = arglist[3]
    if arglen >= 6:
        capwidth = int(arglist[4])
        capheight = int(arglist[5])
    if arglen >= 8:
        targetwidth = int(arglist[6])
        targetheight = int(arglist[7])

    frecognition = FaceRecognition(clientname, ipaddr, port, capwidth, capheight, None, targetwidth,
                                   targetheight, True)
    frecognition.connect()
    frecognition.start()

if __name__ == "__main__":
    main()
