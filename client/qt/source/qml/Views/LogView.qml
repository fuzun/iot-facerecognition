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

import com.iotfacerecognition.log 1.0

import ".."

Item {
    id: logView

    readonly property string title: qsTr("Logs")

    Column {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.top: parent.top
        anchors.margins: logList.anchors.margins

        RowLayout {
            id: headerLayout
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 25

            Label {
                text: qsTr("Timestamp")
                Layout.preferredWidth: tsMetric.width

                Label {
                    id: tsMetric
                    text: "00-00-0000 / 00:00:00"
                    visible: false
                }
            }

            Label {
                id: typeLabel
                text: qsTr("Type")
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("Description")
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right

            height: 2
            color: "lightgray"
            opacity: 0.5
        }
    }

    ListView {
        id: logList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 10

        model: intf.logModel

        clip: true

        spacing: 5

        ScrollBar.vertical: ScrollBar { id: scrollBar; }

        delegate: RowLayout {
            width: logList.width - scrollBar.width

            spacing: headerLayout.spacing

            Label {
                Layout.preferredWidth: tsMetric.width

                text: model.timeStamp
            }

            Image {

                Layout.preferredWidth: typeLabel.width
                Layout.preferredHeight: tsMetric.height

                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter

                source: {
                    switch (model.type) {
                    default:
                    case Log.INFORMATION:
                        return Common.informationIcon
                    case Log.WARNING:
                        return Common.warningIcon
                    case Log.ERROR:
                        return Common.errorIcon
                    }
                }
            }

            Label {
                Layout.fillWidth: true

                wrapMode: Text.Wrap

                text: model.context
            }
        }
    }
}
