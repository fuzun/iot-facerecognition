/*
 * iot-facerecognition-server
 * Copyright (C) 2021 - fuzun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILENAME "config.ini"

#define CONFIG_GUI "GUI"
#define CONFIG_GUI_ENABLE "Enable"
#define CONFIG_GUI_DEFAULT_ENABLE 1

#define CONFIG_CRT "Certificate"
#define CONFIG_CRT_TESTMODE "TestMode"
#define CONFIG_CRT_DEFAULT_TESTMODE 1
#define CONFIG_CRT_KEY_LOCATION "Key"
#define CONFIG_CRT_CRT_LOCATION "Certificate"
#define CONFIG_CRT_DEFAULT_KEY_LOCATION "test.key"
#define CONFIG_CRT_DEFAULT_CRT_LOCATION "test.crt"
#define CONFIG_CRT_KEYALGORITHM "QSsl::KeyAlgorithm"
#define CONFIG_CRT_DEFAULT_KEYALGORITHM 1
#define CONFIG_CRT_ENCODINGFORMAT "QSsl::EncodingFormat"
#define CONFIG_CRT_DEFAULT_ENCODINGFORMAT 0

#define CONFIG_LOG "Log"
#define CONFIG_LOG_ENABLE "Enable"
#define CONFIG_LOG_DEFAULT_ENABLE 1
#define CONFIG_LOG_LOCATION "Location"
#define CONFIG_LOG_DEFAULT_LOCATION "log.txt"

#define CONFIG_DLIB "DLIB"
#define CONFIG_DLIB_FACELANDMARKFILE "FaceLandmarkModelFile"
#define CONFIG_DLIB_DEFAULT_FACELANDMARKFILE "shape_predictor_5_face_landmarks.dat"
#define CONFIG_DLIB_FACERECOGNITIONFILE "FaceRecognitionModelFile"
#define CONFIG_DLIB_DEFAULT_FACERECOGNITIONFILE "dlib_face_recognition_resnet_model_v1.dat"
#define CONFIG_DLIB_THRESHOLD "Threshold"
#define CONFIG_DLIB_DEFAULT_THRESHOLD 0.6
#define CONFIG_DLIB_FACE_DETAILSIZE "FaceDetailSize"
#define CONFIG_DLIB_DEFAULT_FACE_DETAILSIZE 150
#define CONFIG_DLIB_FACE_PADDING "FacePadding"
#define CONFIG_DLIB_DEFAULT_FACE_PADDING 0.25
#define CONFIG_DLIB_REFERENCEFACEFILE "ReferenceFile"
#define CONFIG_DLIB_DEFAULT_REFERENCEFACEFILE "faces.json"
#define CONFIG_DLIB_IMAGENETCLASSIFIERFILE "ImageNetClassifierFile"
#define CONFIG_DLIB_DEFAULT_IMAGENETCLASSIFIERFILE "resnet34_1000_imagenet_classifier.dnn"
#define CONFIG_DLIB_NUMCROPS "NumberOfCrops"
#define CONFIG_DLIB_DEFAULT_NUMCROPS 16

#define CONFIG_CONN "Connection"
#define CONFIG_CONN_SERVERNAME "ServerName"
#define CONFIG_CONN_DEFAULT_SERVERNAME "iot-facerecognition-server"
#define CONFIG_CONN_PORT "Port"
#define CONFIG_CONN_DEFAULT_PORT 50000

#endif // CONFIG_H
