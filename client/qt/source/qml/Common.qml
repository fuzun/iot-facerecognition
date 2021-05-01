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
pragma Singleton

import QtQml 2.12

QtObject {

    function setColorAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    readonly property real toolbarHeight: height / 8

    readonly property string _viewDir: "Views/"
    readonly property string mainView: _viewDir + "MainView.qml"
    readonly property string cameraView: _viewDir + "CameraView.qml"
    readonly property string logView: _viewDir + "LogView.qml"
    readonly property string settingsView: _viewDir + "SettingsView.qml"
    readonly property string cameraSettingsView: _viewDir + "CameraSettingsView.qml"
    readonly property string statisticsView: _viewDir + "StatisticsView.qml"
    readonly property string statisticsSettingsView: _viewDir + "StatisticsSettingsView.qml"


    readonly property real width: 1024
    readonly property real height: 600

    readonly property string title: qsTr("iot-facerecognition qt client")

    readonly property int bgImgTransitionDuration: 250
    readonly property string _bgImgDir: "qrc:/bgImages/"
    readonly property string bgDarkImage: _bgImgDir + "dark.jpg"
    readonly property string bgLightImage: _bgImgDir + "light.jpg"
    readonly property string bgLicense: _bgImgDir + "license.txt"

    readonly property string _iconDir: "qrc:/icons/"
    readonly property string cameraIcon: _iconDir + "Camera.svg"
    readonly property string logIcon: _iconDir + "Log.svg"
    readonly property string settingsIcon: _iconDir + "Settings.svg"
    readonly property string messageIcon: _iconDir + "Message.svg"
    readonly property string aboutIcon: _iconDir + "About.svg"
    readonly property string shutdownIcon: _iconDir + "Shutdown.svg"
    readonly property string informationIcon: _iconDir + "Information.svg"
    readonly property string warningIcon: _iconDir + "Warning.svg"
    readonly property string errorIcon: _iconDir + "Error.svg"
    readonly property string statisticsIcon: _iconDir + "Statistics.svg"
    readonly property string iconLicense: _iconDir + "license.txt"

    readonly property string builtWithQt: _iconDir + "built-with-qt.png"
}
