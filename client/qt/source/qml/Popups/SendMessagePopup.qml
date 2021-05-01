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

import "../Widgets" as Widgets

Widgets.ModalPopup {
    title: qsTr("Send Message")

    context: Component {
        Item {
            id: parentItem
            implicitHeight: childrenRect.height

            signal closePopup()

            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right

                TextArea {
                    id: messageField

                    Layout.preferredHeight: 125
                    Layout.fillWidth: true

                    placeholderText: qsTr("Message...")
                }

                RoundButton {
                    Layout.fillWidth: true
                    text: qsTr("Send!")

                    onClicked: {
                        console.assert(client.connected)

                        client.sendMessage(messageField.text)
                        messageField.clear()
                        parentItem.closePopup()
                    }
                }
            }
        }
    }
}
