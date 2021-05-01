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

import QtGraphicalEffects 1.0

import ".."
import "../Popups"

Item {
    id: mainView

    readonly property string title: Common.title

    signal viewChangeRequest(string view)

    AboutPopup { id: aboutPopup }
    SendMessagePopup { id: sendMessagePopup }

    readonly property real buttonWidth: Math.max(96, root.width / 8.5)
    readonly property real buttonHeight: Math.max(128, root.height / 3.75)

    component ViewButton: Button {
        id: button

        display: AbstractButton.TextUnderIcon
        icon.height: buttonWidth * 0.75
        icon.width: buttonWidth * 0.75

        icon.color: "transparent"

        implicitHeight: buttonHeight
        implicitWidth: buttonWidth

        background: Item {

            RectangularGlow {
                anchors.fill: parent
                glowRadius: button.pressed ? 12 : 2
                spread: 0.2
                color: "#00ABA9"
                visible: button.pressed

                Behavior on glowRadius {
                    NumberAnimation { duration: 75; easing.type: Easing.InOutSine }
                }
            }
            FastBlur {
                anchors.fill: parent

                radius: 48

                source: ShaderEffectSource {
                    readonly property point mappedPoint: Qt.point(button.x, button.y)

                    sourceItem: root.background
                    sourceRect: Qt.rect(mappedPoint.x - flickable.contentX, mappedPoint.y - flickable.contentY + root.header.height, button.width,
                                        button.height)
                }
           }

            Rectangle {
                anchors.fill: parent
                color: Common.setColorAlpha(bgColor, 0.1)

                readonly property color bgColor: root.isThemeDark ? "darkgray" : "lightgray"
                readonly property color borderColor: bgColor //"#00ABA9"

                border.width: 2
                border.color: Common.setColorAlpha(borderColor, 0.85)
            }

            Rectangle {
                anchors.fill: parent
                color: "lightgray"

                opacity: 0.5

                visible: button.pressed
            }
        }
    }

    Image {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: width * 0.25
        anchors.bottomMargin: anchors.rightMargin

        asynchronous: true
        antialiasing: true

        width: implicitWidth * 0.175
        height: implicitHeight * 0.175

        source: Common.builtWithQt
    }

    Flickable {
        id: flickable
        anchors.fill: parent

        contentWidth: width
        contentHeight: flow.height

        ScrollBar.vertical: ScrollBar {
            visible: flickable.contentHeight > flickable.height
            policy: ScrollBar.AlwaysOn
        }

        Flow {
            id: flow
            anchors.left: parent.left
            anchors.right: parent.right

            padding: buttonWidth / 2
            spacing: padding

            ViewButton {
                text: "Camera"

                icon.source: Common.cameraIcon

                onClicked: {
                    viewChangeRequest(Common.cameraView)
                }
            }

            ViewButton {
                text: "Logs"

                icon.source: Common.logIcon

                onClicked: {
                    viewChangeRequest(Common.logView)
                }
            }

            ViewButton {
                text: "Settings"

                icon.source: Common.settingsIcon

                onClicked: {
                    viewChangeRequest(Common.settingsView)
                }
            }

            ViewButton {
                enabled: client.connected

                text: "Send\nMessage"

                icon.source: Common.messageIcon

                onClicked: {
                    sendMessagePopup.open()
                }
            }

            ViewButton {
                text: qsTr("Statistics")

                icon.source: Common.statisticsIcon

                onClicked: {
                    viewChangeRequest(Common.statisticsView)
                }
            }

            ViewButton {
                text: "About"

                icon.source: Common.aboutIcon

                onClicked: {
                    aboutPopup.open()
                }
            }

            ViewButton {
                text: "Shutdown"

                icon.source: Common.shutdownIcon

                onClicked: {
                    Qt.quit()
                }
            }
        }
    }
}
