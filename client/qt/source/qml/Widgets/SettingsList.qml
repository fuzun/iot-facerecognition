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

import com.iotfacerecognition.settings 1.0

Item {
    id: settingsList

    property alias model: sectionRepeater.model

    component BaseSetting: Item {
        id: baseSetting
        implicitHeight: childrenRect.height

        property var modelData: undefined

        property Component inputComponent: undefined
        property real inputComponentMinimumWidth: 5
        property real inputComponentPreferredWidth: 150

        signal focusRequest()

        Connections {
            target: root.inputPanel

            function onActiveChanged() {
                if (root.inputPanel.active && inputLoader.item.focusIndicator)
                    focusRequest()
            }
        }

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            height: implicitHeight + implicitHeight / 2

            Label {
                Layout.fillWidth: true

                wrapMode: Text.Wrap
                text: modelData.description
            }

            Loader {
                id: inputLoader

                Layout.preferredWidth: inputComponentPreferredWidth
                Layout.minimumWidth: inputComponentMinimumWidth

                sourceComponent: inputComponent
            }
        }
    }

    Component {
        id: textFieldSetting

        BaseSetting {
            inputComponent: RowLayout {
                property bool focusIndicator: textField.activeFocus

                spacing: 5

                function send() {
                    modelData.value = textField.text
                    textField.isTextEdited = false
                }

                TextField {
                    id: textField

                    Layout.fillWidth: true

                    property bool isTextEdited: false

                    text: modelData.value

                    IntValidator {
                        id: intValidator
                        bottom: modelData.ctx[modelData.keyMin] ?? -2147483647
                        top: modelData.ctx[modelData.keyMax] ?? 2147483647
                    }

                    validator: (modelData.ctx[modelData.keyType] === SettingObject.TextFieldInt) ? intValidator : null

                    onTextEdited: {
                        isTextEdited = true
                    }

                    onActiveFocusChanged: {
                        if (activeFocus)
                            focusRequest()
                    }

                    Keys.onReturnPressed: {
                        send()
                        event.accepted = true
                    }
                }

                Button {
                    id: okButton
                    text: qsTr("OK")
                    visible: textField.isTextEdited

                    onClicked: {
                        send()
                    }
                }
            }
        }
    }

    Component {
        id: sliderSetting

        BaseSetting {
            inputComponent: Component {
                Slider {
                    id: slider

                    readonly property var minVal: modelData.ctx[modelData.keyMin]
                    readonly property var maxVal: modelData.ctx[modelData.keyMax]

                    function normalize(val) {
                        return ((val - minVal) / (maxVal - minVal))
                    }

                    function deNormalize(val) {
                        return Math.round((val * (maxVal - minVal) + minVal))
                    }

                    value: normalize(modelData.value)

                    onMoved: {
                        modelData.value = deNormalize(value)
                    }

                    ToolTip {
                        parent: slider.handle
                        visible: slider.pressed
                        text: slider.deNormalize(slider.value)
                    }
                }
            }
        }
    }

    Component {
        id: toggleSetting

        BaseSetting {
            inputComponent: Component {
                Switch {
                    checked: modelData.value

                    onReleased: {
                        modelData.value = checked
                    }
                }
            }
        }
    }

    Component {
        id: dropDownSetting

        BaseSetting {
            inputComponent: Component {
                ComboBox {
                    model: modelData.ctx[modelData.keyDropDownCtx]

                    currentIndex: modelData.value

                    popup.onClosed: {
                        modelData.value = currentIndex
                    }
                }
            }
        }
    }

    Component {
        id: buttonSetting

        BaseSetting {
            inputComponent: Component {
                Button {
                    text: modelData.text

                    onClicked: {
                        modelData.value = null
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Common.setColorAlpha(bgColor, 0.15)

        readonly property color bgColor: root.isThemeDark ? "darkgray" : "lightgray"
    }


    Component {
        id: sectionComponent

        Column {
            id: section
            property alias model : settingsList.model
            property string title

            component Separator : Rectangle {
                color: root.isThemeDark ? "darkgray" : "lightgray"
                height: 1
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right

                height: 50

                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter

                text: title
                // font.weight: Font.DemiBold
            }

            Separator {
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Repeater {
                id: settingsList

                anchors.left: parent.left
                anchors.right: parent.right

                delegate: Item {
                    id: itemDelegate

                    width: settingsList.width - scrollBar.width

                    implicitHeight: childrenRect.height

                    visible: !!loader.sourceComponent

                    property var modelData: settingsList.model[index]

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right

                        Rectangle {

                            visible: index !== 0
                            anchors.left: parent.left
                            anchors.right: parent.right

                            color: root.isThemeDark ? "darkgray" : "lightgray"
                            height: 1
                        }

                        Loader {
                            id: loader
                            anchors.left: parent.left
                            anchors.right: parent.right

                            anchors.leftMargin: 8
                            anchors.rightMargin: 8

                            sourceComponent: {
                                switch (modelData.ctx[modelData.keyType]) {
                                case SettingObject.TextField:
                                case SettingObject.TextFieldInt:
                                    return textFieldSetting
                                case SettingObject.ToggleButton:
                                    return toggleSetting
                                case SettingObject.DropDown:
                                    return dropDownSetting
                                case SettingObject.Slider:
                                    return sliderSetting
                                case SettingObject.Button:
                                    return buttonSetting
                                }
                            }

                            onLoaded: {
                                loader.item.modelData = Qt.binding(function() { return itemDelegate.modelData })
                            }

                            Connections {
                                target: loader.item

                                ignoreUnknownSignals: true

                                function onFocusRequest() {
                                    const localY = itemDelegate.y + itemDelegate.height + section.parent.y
                                    const y = flickable.mapToItem(root.background, flickable.x, localY).y

                                    if (y > root.inputPanelY) {
                                        flickable.contentY = (y - root.inputPanelY + itemDelegate.height)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Separator {
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }
    }


    Flickable {
        id: flickable
        anchors.fill: parent
        ScrollBar.vertical: ScrollBar { id: scrollBar }

        contentWidth: width
        contentHeight: column.height

        Behavior on contentY {
            NumberAnimation { duration: 125; easing.type: Easing.InOutSine }
        }

        Column {
            id: column
            anchors.left: parent.left
            anchors.right: parent.right

            Repeater {
                id: sectionRepeater
                anchors.left: parent.left
                anchors.right: parent.right

                model: []

                Loader {
                    id: sectionLoader
                    anchors.left: parent.left
                    anchors.right: parent.right

                    sourceComponent: sectionComponent

                    onLoaded: {
                        item.model = Qt.binding(function() { return modelData.model; })
                        item.title = Qt.binding(function() { return modelData.title; })
                    }
                }
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                height: root.inputPanelVisible ? root.inputPanelHeight : 0
            }
        }
    }
}
