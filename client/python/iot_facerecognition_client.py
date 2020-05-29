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


class FaceRecognition:
    def __init__(self, clientname, ipaddress, port, capturewidth, captureheight, callbackfunc=None,
                 targetwidth=0, targetheight=0, lessbandwidth=False, guienabled=False,
                 enablewstrace=False, allowselfsign=True):
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
        self.allowselfsign = allowselfsign

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
        if self.allowselfsign:
            self.ws = websocket.WebSocket(sslopt={"cert_reqs": ssl.CERT_NONE})
        else:
            self.ws = websocket.WebSocket()
        self.ws.connect("wss://" + str(self.ipaddress) + ":" + str(self.port))
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
        if self.guienabled:
            cv2.namedWindow('facerecognition', cv2.WND_PROP_FULLSCREEN)
            cv2.setWindowProperty('facerecognition', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
            screen = screeninfo.get_monitors()[0]
            self.guiwidth, self.guiheight = screen.width, screen.height
            self.nFactorW = float(self.guiwidth) / float(self.targetwidth)
            self.nFactorH = float(self.guiheight) / float(self.targetheight)

        while not self.stop:
            ret, frame = self.cap.read()
            frame2 = frame

            if self.targetwidth > 0 and self.targetheight > 0 and \
                    self.targetheight != self.captureheight and self.targetwidth != self.capturewidth:
                frame = cv2.resize(frame, (self.targetwidth, self.targetheight), interpolation=cv2.INTER_AREA)

            if self.guienabled and abs(self.guiwidth - self.capturewidth) > 5 \
                    and abs(self.guiheight - self.captureheight) > 5:
                frame2 = cv2.resize(frame2, (self.guiwidth, self.guiheight))

            if self.lessBandwidth is False:
                ret2, encoded = cv2.imencode('.jpg', frame)
            else:
                ret2, encoded = cv2.imencode('.webp', frame)

            self.ws.send_binary(encoded.tobytes())

            if self.guienabled:
                if time.time() - self.time < 1.5 and self.msg is not None:
                    for msg in self.msg.split(':'):
                        pmsg = msg.split(',')
                        if self.callbackfunc is not None and self.cbackcalled is False and str(pmsg[4]) != '':
                            self.callbackfunc(str(pmsg[4]))
                            self.cbackcalled = True
                        x0 = int(pmsg[0]) * self.nFactorW
                        x1 = int(pmsg[2]) * self.nFactorW
                        y0 = int(pmsg[1]) * self.nFactorH
                        y1 = int(pmsg[3]) * self.nFactorH
                        cv2.rectangle(frame2, (int(x0), int(y0)), (int(x1), int(y1)),
                                              (int(128), int(0), int(0)), 3)
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        cv2.putText(frame2, str(pmsg[4]), (int(x0), int(y1 + 25)), font, 1.5, (0, 0, 200), 2,
                                    cv2.LINE_AA)
                cv2.imshow('facerecognition', frame2)
                cv2.waitKey(35)  # ~30 FPS

    def receiveloop(self):
        while not self.stop:
            self.msg = self.ws.recv()
            self.cbackcalled = False
            self.time = time.time()

    def sendmessage(self, message):
        self.ws.send("1:" + message)


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
    lessbandwidth = False

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
    if arglen >= 9:
        lessbandwidth = bool(arglist[8])

    frecognition = FaceRecognition(clientname, ipaddr, port, capwidth, capheight, None, targetwidth,
                                   targetheight, lessbandwidth, True)
    frecognition.connect()
    frecognition.start()

if __name__ == "__main__":
    main()
