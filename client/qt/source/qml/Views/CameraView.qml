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

import com.iotfacerecognition.components 1.0

import ".."
import "../Widgets" as Widgets

Item {
    id: cameraView

    readonly property bool backgroundVisible: false
    readonly property bool toolbarVisible: false

  //  Universal.theme: Universal.Dark
  //  Universal.background: Universal.Steel
  //  Universal.foreground: Universal.Cyan

    signal viewChangeRequest(string view)

    Component.onCompleted: {
        camera.start()
    }

    Component.onDestruction: {
        camera.stop()
    }

    YUVRendererWidget {
        anchors.fill: parent

        frame: camera.currentFrame

        Connections {
            target: camera

            function onFrameBufferUpdated() {
                root.update()
            }
        }
    }

    Popup {
        id: statisticsPopup

        implicitWidth: parent.width / 1.25
        implicitHeight: parent.height / 1.25

        contentWidth: implicitWidth * 0.85
        contentHeight: implicitHeight * 0.85

        anchors.centerIn: parent

        clip: true

        modal: true

        opacity: 0.85

        contentItem: Loader {
            active: statisticsPopup.visible

            asynchronous: true

            source: "StatisticsView.qml"
        }
    }

    component CustomLabel : Label {
        background: Rectangle {
            opacity: 0.75
            color: camera.settings.accentColor
        }

        font.pointSize: camera.settings.labelPointSize

        color: "white"
    }

    CustomLabel {
        id: objectListLabel
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        Loader {
            id: objectListCleaner
            active: (camera.settings.objectListDuration >= 0)

            function restart() {
                item.restart()
            }

            sourceComponent: Timer {
                interval: camera.settings.objectListDuration
                repeat: true

                running: true
                onTriggered: objectListLabel.text = ""
            }
        }

        Connections {
            target: client

            function onObjectTagReceived(list) {
                objectListCleaner.restart()

                objectListLabel.text = ""
                for (let i = 0; i < list.length; ++i) {
                    objectListLabel.text += "%1: %2\n".arg(list[i].prediction)
                                                      .arg(list[i].label)
                }
                objectListLabel.text = objectListLabel.text.slice(0, -1)
            }
        }
    }

    CustomLabel {
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        text: "FPS: %1".arg(camera.fps)
        visible: camera.settings.showFPS
    }

    CustomLabel {
        id: trafficInfo
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        visible: camera.settings.showTotalInternetUsage

        Timer {
            repeat: true
            interval: 1000
            running: true

            onTriggered: {
                const totalSent = qsTr("Total Sent: %3 mbytes").arg(client.totalSentBytes / 1024.0 / 1024.0)
                const totalReceived = qsTr("Total Received: %4 kbytes").arg(client.totalReceivedBytes / 1024.0)

                trafficInfo.text = totalSent + "\n" + totalReceived
            }
        }
    }


    Canvas {
        id: canvas

        anchors.fill: parent

        property var faceList: []

        property var _faceTagLabelList: []
        property bool _faceTagLabelsUpdated: false

        onFaceListChanged: {
            canvas._faceTagLabelList.forEach((item, index, array) => { item.destroy() })
            canvas._faceTagLabelList = []

            canvas._faceTagLabelsUpdated = false
            requestPaint()

            faceListCleaner.restart()
        }

        Component {
            id: faceTagLabel

            CustomLabel { }
        }

        Connections {
            target: client

            function onFaceTagReceived(list) {
                canvas.faceList = list
            }
        }

        Loader {
            id: faceListCleaner
            active: camera.settings.faceListDuration >= 0

            function restart() {
                if (status === Loader.Ready)
                    item.restart()
            }

            sourceComponent: Timer {
                interval: camera.settings.faceListDuration
                repeat: true

                onTriggered: {
                    canvas.faceList = []
                }
            }
        }

        onPaint: {
            const ctx = getContext("2d");

            ctx.clearRect(0, 0, width, height)

            ctx.strokeStyle = camera.settings.accentColor
            ctx.lineWidth = camera.settings.lineWidth

            const widthScale = root.width / camera.frameSize().width
            const heightScale = root.height / camera.frameSize().height

            for (let i = 0; i < faceList.length; ++i) {
                const obj = faceList[i]
                const rect = Qt.rect(obj.x, obj.y, obj.width, obj.height)

                const x = rect.x * widthScale
                const y = rect.y * heightScale
                const _width = rect.width * widthScale
                const _height = rect.height * heightScale
                ctx.strokeRect(x, y, _width, _height)


                if (!_faceTagLabelsUpdated) {
                    const label = faceTagLabel.createObject(canvas, {
                        text: obj.tag,
                        x: x,
                        y: y + _height
                    });

                    _faceTagLabelList.push(label)
                }
            }

            _faceTagLabelsUpdated = true
        }
    }

    RoundButton {
        id: backButton

        anchors {
            top: parent.top
            left: parent.left
            topMargin: height / 2
            leftMargin: width / 2
        }


        text: qsTr("‹")

        implicitHeight: settingsButton.implicitHeight
        implicitWidth: settingsButton.implicitWidth

        onClicked: viewChangeRequest(Common.mainView)
    }

    Column {
        anchors {
            top: parent.top
            right: parent.right
            topMargin: backButton.anchors.topMargin
            rightMargin: width / 2
        }

        spacing: 5

        RoundButton {
            id: settingsButton

            text: qsTr("⚙")
            implicitHeight: implicitWidth

            onClicked: viewChangeRequest(Common.cameraSettingsView)
        }

        RoundButton {
            id: statisticsButton

            text: qsTr("📊")
            implicitHeight: implicitWidth

            onClicked: {
                if (statisticsPopup.opened)
                    statisticsPopup.close()
                else
                    statisticsPopup.open()
            }
        }
    }
}
