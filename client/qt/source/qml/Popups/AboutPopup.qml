/*
 * iot-facerecognition-client-qt
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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.15

import "../Widgets" as Widgets
import ".."

Widgets.ModalPopup {
    title: "About"

    context: Component {
        Label {
            id: aboutLabel

            textFormat: Text.StyledText

            wrapMode: Text.Wrap
            text: "<h3>" + Common.title + "</h3>" +
                  "<ul>" +
                  "<li>Address: github/fuzun/iot-facerecognition</li>" +
                  "<li>License: GNU Affero GPL</li>" +
                  "<li>Author: fuzun</li>" +
                  "<li>Version: " + Qt.application.version + "</li>" +
                  "</ul>" +

                  "<h3>Used Open Source Projects: </h3><br/>" +

                  "<h4>Qt Framework Base</h4>" +
                  "<ul>" +
                  "<li>Address: https://github.com/qt/qtbase" +
                  "<li>Used under GNU LGPL Version 3 license</li>" +
                  "</ul>" +

                  "<h4>qtvirtualkeyboard</h4>" +
                  "<p>Qt Module - QtQuick virtual keyboard</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com/qt/qtvirtualkeyboard" +
                  "<li>Used under GNU GPL Version 3 license</li>" +
                  "</ul>" +

                  "<h4>qtcharts</h4>" +
                  "<p>Qt Module - QtCharts module</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com/qt/qtcharts" +
                  "<li>Used under GNU GPL Version 3 license</li>" +
                  "</ul>" +

                  "<h4>qtwebsockets</h4>" +
                  "<p>Qt Module - Qt WebSockets</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com/qt/qtwebsockets" +
                  "<li>Used under GNU LGPL Version 3 license</li>" +
                  "</ul>" +

                  "<h4>qtquickcontrols2</h4>" +
                  "<p>Qt Module - Qt Quick Controls 2</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com/qt/qtquickcontrols2" +
                  "<li>Used under GNU LGPL Version 3 license</li>" +
                  "</ul>" +

                  "<h4>OpenMaxIL-cpp</h4>" +
                  "<p>OpenMax IL C++ wrapper for RaspberryPi </p>" +
                  "<ul>" +
                  "<li>Address: https://github.com//dridri/OpenMaxIL-cpp" +
                  "<li>Used under MIT License</li>" +
                  "</ul>" +

                  "<h4>SampleYUVRenderer</h4>" +
                  "<p>Very basic sketch of rendering YUV frames via Qt/OpenGL</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com//MasterAler/SampleYUVRenderer" +
                  "<li>Used under MIT License</li>" +
                  "</ul>" +

                  "<h4>Qt Official Docs</h4>" +
                  "<p>Scene Graph - OpenGL Under QML</p>" +
                  "<ul>" +
                  "<li>Address: https://doc-snapshots.qt.io/qt5-5.15/qtquick-scenegraph-openglunderqml-example.html" +
                  "<li>Used under BSD License</li>" +
                  "</ul>" +

                  "<h4>yuv2rgb</h4>" +
                  "<p>C99 library for fast image conversion between yuv420p and rgb24</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com/descampsa/yuv2rgb" +
                  "<li>Used under BSD-3-Clause License</li>" +
                  "</ul>" +

                  "<h4>yuv2rgb</h4>" +
                  "<p>C99 library for fast image conversion between yuv420p and rgb24</p>" +
                  "<ul>" +
                  "<li>Address: https://github.com/descampsa/yuv2rgb" +
                  "<li>Used under BSD-3-Clause License</li>" +
                  "</ul>" +

                  "<h4>libjpeg-turbo</h4>" +
                  "<ul>" +
                  "<li>Address: https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master" +
                  "<li>Used under The Modified (3-clause) BSD License</li>" +
                  "</ul>" +

                  "<h4>Background Images</h4>" +
                  intf.readTextFile(Common.bgLicense) +

                  "<h4>Icons</h4>" +
                  intf.readTextFile(Common.iconLicense)
        }
    }
}
