# NOTE:

<p>This project is currently in maintenance mode and it is not suggested to use except for debug purposes. The README is somewhat obsolote and subject to change. The biggest change will be a brand new client made with Qt Framework. The new client will come with a fluid UI made with QML and it will use OpenMAX to grasp camera images to be able to run with high performance in embedded devices such as Raspberry Pi Zero W.</p>

# iot-facerecognition
<p>iot-facerecognition is a project that was created to provide real-time face detection ability to iot devices such as Raspberry Pi that are not normally able to recognize faces due to low computing power. The logic behind is to offload the work required of recognizing faces to a capable back-end solution in a simple way. Websocket is used for data transmission for versatility. SSL is forced enable to secure the connection.</p>

<p>iot-facerecognition consists of two parts: server and client. Server, or the backend, is responsible for parsing the data that the connected clients send concurrently. It uses DLIB and OpenCV to perform face detection and recognition. Resources are allocated to the clients evenly.  It does the face recognition for each client in parallel. It is fast and it is designed to handle many amount of clients. It is written in modern C++.</p>

![Schematic 2](https://raw.githubusercontent.com/fuzun/iot-facerecognition/master/schematic2.png)<p>The client captures image sequence (30~ FPS) and streams it to the server, continuously. After the server resolves the faces contained in a certain video frame it sends data regarding the faces (Geometry and tag) to the client and according to the client configuration, client shows rectangle around faces along with their tags.</p>

Currently, for the client side, there is a Python based implementation which was published just  recently. Different client implementations are expected to be developed later on.

## Installation

### Server
iot-facerecognition-server is written in C++ and it uses Qt Framework. Qt 5.15 and MSVC19 is used for Windows releases (precompiled binaries).

#### Build From Sources

<p>The server uses OpenCV and DLIB. So in order to build iot-facerecognition-server, along with these libraries, depending on the DLIB build configuration, additional libraries such as BLAS, LAPACK and/or CUDA may be needed. There is a .pro file for quickly setting up a user configuration. You must edit the parts if needed. For DLIB, there must be a DLIB_DIR environment variable pointing DLIB library installation. Currently CUDA support is enabled by default. If you want to use DLIB without CUDA support, you should comment the "CONFIG  +=  CUDA_ENABLED" line in the .pro file. On the page: https://docs.nvidia.com/deeplearning/sdk/cudnn-install/index.html, you can check out how to install required libraries regarding CUDA. Make sure CUDA_PATH points to the correct directory. As for OpenCV, currently, it is set to use the latest OpenCV library available for MSVC (https://github.com/opencv/opencv/releases). If you want to link against OpenCV, make sure that "OPENCV_SDK_DIR" environment variable points to the correct directory. In my case it is "...\opencv\build". Set "OCV_COMPILER_SPECIFIC" when needed.</p>
<p>After setting up OpenCV, you can directly build the project. No extra configuration is needed. However, please make sure to copy the reference <b>config.ini</b> file to the build directory.</p>
<p>Note that DLIB works the best when it is compiled in release mode. Make sure to compile it in release mode with x64 architecture. Otherwise it will be very slow to detect faces.</p>

#### Using precompiled binaries
<p>Currently, a version that was compiled for x64 Windows system is available in the releases section. Note that it was not linked statically so even though Qt libraries and the OpenCV library are added to the archive, MSVC runtimes are not added so if you get any error upon launching the application make sure that you have latest MSVC runtimes installed on your system.</p>
<p>Depending on DLIB configuration, CUDA / CUDNN runtimes may be needed. Please read the description of the regarding release carefully. They point out the needed external libraries.</p>
<p>There is no setup procedure as of now. Just extract the archive and launch the server there.</p>

### Client

#### Python
Python client is now available on PyPI! : https://pypi.org/project/iot-facerecognition-client
You may install through PyPI as such:
```
pip install iot-facerecognition-client
```
Alternatively, you can apply standard Python application installation routines. opencv-python, websocket-client and screeninfo packages are needed for installation.
Please note that on Raspberry PI, the latest OpenCV-python seems to be broken so you may need to download a specific version that is older than the current one.

## Usage
### Server
<p>The server needs to be configured before usage. To configure the server, please open the <b>config.ini</b> file that comes with the server and modify lines according to these definitions:</p>

* GUI
  * Enable = (boolean) -> You can disable or enable the GUI using this entry. Disabling GUI reserves system resources to be allocated for more clients. 0 Disables 1 Enables. Default is 1.
* Certificate
  * TestMode = (boolean) -> If set 0, server will throw an exception when SSL error occurs. Default is 1.
  * Key = (string) -> Specifies the file location for private key file (*.key). A sample key file is provided but should not be used in production. Default points to the sample key file.
  * Certificate = (string) -> Specifies the file location for public certificate file (*.cert, *.crt). Certificate hostname must match with the listening ip address / hostname. A sample certificate file is provided but should not be used in production. Default points to the sample cert file.
  * QSsl::KeyAlgorithm = (int) -> Set it according to the information available in https://doc.qt.io/qt-5/qssl.html#KeyAlgorithm-enum. Default is 1 (RSA Algorithm).
  * QSsl::EncodingFormat = (int) -> Set it according to the information available in https://doc.qt.io/qt-5/qssl.html#EncodingFormat-enum. Default is 0 (PEM Format).
* Log
  * Enable = (boolean) -> Enables or disables logging to file. Default is 1.
  * Location = (string) -> Points to the file location in which the logs are stored. Default is "log.txt".
* DLIB
  * ReferenceFile = (string) -> Specifies the file containing reference face photos and their respective tags. iot-facerecognition-server will try to find faces that are specified in this file in the input stream. Default is "face.txt". This file should have a structure as such: <p>tag1 : photo1.jpg<br/>tag2 : photo2.jpg</p>
  * FaceLandmarkModelFile = (string) -> Specifies the face landmark model file for DLIB. Can be gathered from https://github.com/davisking/dlib-models. Default is "shape_predictor_5_face_landmarks.dat"
  * FaceRecognitionModelFile = (string) -> Specifies the face recognition model file for DLIB. Can be gathered from https://github.com/davisking/dlib-models. Default is "dlib_face_recognition_resnet_model_v1.dat"
  * Threshold = (double) -> Threshold for face matching. Should probably not be changed. Default is 0.6.
  * FaceDetailSize = (int) -> DLIB internal. Should probably not be changed. Default is 150.
  * FacePadding = (int) -> DLIB internal. Should probably not be changed. Default is 0.25.
* Connection
  * ServerName = (string) -> Name of the server. Default is "iot-facerecognition-server".
  * Port = (int) -> Specifies the port of the Websocket to listen on. This port should not be used by other applications. Default is 50000.

After configuring the server, you can directly run it. If GUI is enabled, a gui will be shown along with console. In this gui you can see the clients that are connected. Double clicking a client will open a dialog box in which the input video stream is shown and faces are marked with a rectangle and label (label text is the tag associated with that face).

When a known or unknown face is found, iot-facerecognition-server will send the face data to the client. If an unknown face is found the face tag will be empty string.

### Client
#### Python
<p>First of all, if allowselfsign parameter (described below) is False you need to make sure that the certificate that the server is configured with is installed as a trusted certificate on all clients otherwise you will get SSL errors.</p>
In the iot-facerecognition-client.py file there is a class named "FaceRecognition". This class handles all the things regarding server connection and will call a callback function when a known face is found in the capture stream thus client implementations are expected to be used by an external application. The external application should create an instance of this class with the following parameters:</p>

* clientname = (string) -> Name of the client. Server will use this name to list and identify the clients.
* ipaddress = (string) -> ip address of the server.
* port = (string) -> port number that is being listened by the server.
* capturewidth, captureheight = (int) -> Capture width & height. Advised to be set according to the maximum resolution that the capture device supports.
* callbackfunc = (function(str)) -> This function will be called when a known face is found. A string argument containing the tag will be passed to the function.
* targetwidth, targetheight = (int) -> Target width & height. Captured frame will be resized to this size before sending it to the server. Advised to be capture resolution / 2. Should be configured according to the bandwidth available. If is 0, resize will not occur.
* lessbandwidth = (boolean) -> Sets if frame is to be encoded as JPEG or WEBP format. WEBP format consumes less bandwidth but takes more time to encode. Consider this tradeoff and set it according to your system. False means JPEG will be used, True means WEBP will be used. Default is False.
* guienabled = (boolean) -> Enables or disables the GUI. Disabling the GUI will make it virtually a blackbox which invokes callback function whenever faces are detected. When gui is enabled, camera stream will always be shown fullscreen. Default is False.
* enablewstrace = (boolean) -> Enables or disables websocket tracing. Enabling it will make it log every websocket related activity to the console. Very time consuming activity. Default is False.
* allowselfsign = (boolean) -> Allows certificate to be self signed. Default is True.

Example usage:
```python
from iot_facerecognition_client import FaceRecognition

def callback(tag):
	print("Found Face: " + tag)

if __name__ == "__main__":
  facerecognition = FaceRecognition(parameters mentioned above, callbackfunc = callback)
  facerecognition.connect()
  facerecognition.start()
```

<p>Alternatively, python client can be used as a standalone application (with console command 'iot-facerecognition-client'). Although it will not invoke the callback function, it will show GUI and show detected faces. It is also useful for debugging purposes. In this mode, the client should be started by specifying arguments.</p>
Example usage:</p>

```
iot-facerecognition-client clientname ipaddr port capwidth capheight targetwidth targetheight lessbandwidth
```

## Demo
### Server
![Server configuration](https://raw.githubusercontent.com/fuzun/iot-facerecognition/master/demo1.png)
![Server GUI 1](https://raw.githubusercontent.com/fuzun/iot-facerecognition/master/demo2.png)
![Server GUI 2](https://raw.githubusercontent.com/fuzun/iot-facerecognition/master/demo3.png)
### Client (optional gui)
![Client GUI](https://raw.githubusercontent.com/fuzun/iot-facerecognition/master/demo4.png)
## License
iot-facerecognition server and client implementations are all licensed under GPLv3. It can <b>not</b> be used for any commercial purposes. If you need another licensing solution, please contact with me. 
### Qt
Qt is licensed under LGPLv3.
### DLIB
Boost Software License - Version 1.0 - August 17th, 2003
### OpenCV
BSD License. See https://opencv.org/license
