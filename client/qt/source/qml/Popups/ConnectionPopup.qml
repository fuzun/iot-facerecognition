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

import Qt.labs.settings 1.1

import "../Widgets" as Widgets


Widgets.ModalPopup {
    title: "Connection"

    widthRate: 2
    heightRate: 1.5

    context: Component {
        Item {
            id: parentItem

            implicitHeight: childrenRect.height

            Settings {
                id: connectionSettings

                property alias serverAddress: hostField.text
                property alias port: portField.text
                property alias autoConnect: autoConnectToggle.checked

                Component.onCompleted: {
                    if (autoConnect)
                        connect()
                }
            }

            function connect() {
                client.connect("wss://%1:%2".arg(hostField.length > 0 ? hostField.text : hostField.placeholderText)
                                            .arg(portField.length > 0 ? portField.text : portField.placeholderText))
            }

            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right

                GridLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    columns: 2

                    enabled: !client.connected

                    Label {
                        text: qsTr("Host Address:")
                    }

                    TextField {
                        id: hostField
                        implicitWidth: ipMetric.width

                        placeholderText: "127.0.0.1"

                        TextMetrics {
                            id: ipMetric
                            text: "   255.255.255.255   "
                        }
                    }

                    Label {

                        text: qsTr("Port:")
                    }

                    TextField {
                        id: portField
                        placeholderText: "50000"

                        validator: IntValidator {bottom: 0; top: 65535}
                    }

                    Label {
                        text: qsTr("URL:")
                    }

                    Label {
                        text: qsTr("wss://%1:%2").arg(hostField.length > 0 ? hostField.text : hostField.placeholderText)
                                                 .arg(portField.length > 0 ? portField.text : portField.placeholderText)
                    }
                }

                CheckBox {
                    id: autoConnectToggle
                    Layout.fillWidth: true
                    text: qsTr("Auto connect")
                }

                RoundButton {
                    Layout.fillWidth: true
                    text: client.connected ? qsTr("DISCONNECT") : qsTr("CONNECT!")

                    onClicked: {
                        if (client.connected)
                            client.disconnect()
                        else
                            connect()
                    }
                }
            }
        }
    }
}
