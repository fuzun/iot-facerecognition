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
import QtQuick.Layouts 1.12

import QtGraphicalEffects 1.0

import ".."

Popup {
    id: popup

    modal: true

    x: root.width / 2 - width / 2
    y: root.height / 2 - height / 2 - root.header.height

    property real widthRate: 1.5
    property real heightRate: 1.35

    width: root.width / widthRate
    height: root.height / heightRate

    property string title
    property Component context: undefined

    Component.onCompleted: {
        console.assert(context != null)
    }

    Connections {
        target: root.inputPanel

        function onActiveChanged() {
            const diff = (popup.y + root.header.height + popup.height - (root.inputPanelY))
            if (root.inputPanel.active)
                popup.y -= diff
            else
                popup.y = Qt.binding(function() { return root.height / 2 - height / 2 - root.header.height })
        }
    }

    contentItem: Item {

        ColumnLayout {
            anchors.fill: parent

            implicitWidth: childrenRect.width
            implicitHeight: childrenRect.height

            Label {
                id: titleLabel
                Layout.fillWidth: true
                text: title
                horizontalAlignment: Text.AlignHCenter

                visible: title.length > 0
            }

            Rectangle {
                Layout.fillWidth: true
                height: 2
                color: "lightgray"
                opacity: 0.5

                visible: titleLabel.visible
            }

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true

                readonly property bool contentExceedsNaturalHeight: ctxLoader.implicitHeight > height

                contentWidth: width
                contentHeight: contentExceedsNaturalHeight ? ctxLoader.implicitHeight : height

                ScrollBar.vertical: ScrollBar { id: scrollBar; visible: parent.contentExceedsNaturalHeight; width: visible ? implicitWidth : 0 }

                clip: true

                Loader {
                    id: ctxLoader

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: scrollBar.width
                    anchors.verticalCenter: parent.verticalCenter

                    sourceComponent: context

                    Connections {
                        target: ctxLoader.item

                        ignoreUnknownSignals: true

                        function onClosePopup() {
                            popup.close()
                        }
                    }
                }
            }

            RoundButton {
                Layout.fillWidth: true

                text: "Close"

                onClicked: {
                    popup.close()
                }
            }
        }
    }

    background: Item {
        FastBlur {
            id: bgBlur

            anchors.fill: parent

            radius: 0

            Behavior on radius {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InSine
                }
            }

            Connections {
                target: popup

                function onVisibleChanged() {
                    if (popup.visible) {
                        if (!root.background.isBlurred)
                            bgBlur.radius = 64
                    } else {
                        bgBlur.radius = 0
                    }
                }
            }

            source: ShaderEffectSource {
                sourceItem: root.background
                sourceRect: Qt.rect(popup.x, popup.y + root.header.height, popup.width,
                                    popup.height)
            }
        }

        Rectangle {
            anchors.fill: parent
            color: Common.setColorAlpha(bgColor, 0.15)

            readonly property color bgColor: root.isThemeDark ? "darkgray" : "lightgray"
            readonly property color borderColor: bgColor //"#00ABA9"

            border.width: 2
            border.color: Common.setColorAlpha(borderColor, 0.85)
        }
    }
}
