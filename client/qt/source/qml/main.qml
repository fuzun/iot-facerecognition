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

import QtQuick.VirtualKeyboard 2.15
import QtGraphicalEffects 1.0

import QtQml 2.15 // for Binding { }

import "Popups"

ApplicationWindow {
    id: root
    width: Common.width
    height: Common.height
    visible: true
    title: Common.title

    Universal.theme: intf.settings.darkMode ? Universal.Dark : Universal.Light
    Universal.accent: Universal.Teal

    readonly property bool isThemeDark: Universal.theme === Universal.Dark ? true : false

    readonly property bool inputPanelVisible: inputPanel.state === "visible"
    readonly property real inputPanelY: root.height - inputPanel.height
    property alias inputPanelHeight: inputPanel.height
    property alias inputPanel: inputPanel

    signal resetButtonClicked()
    signal settingsButtonClicked()

    ConnectionPopup { id: connectionPopup }

    header: ToolBar {
        id: toolBar

       // height: Common.toolbarHeight

        background: Item {
            FastBlur {
                anchors.fill: parent

                radius: 64

                source: ShaderEffectSource {
                    sourceItem: root.background
                    sourceRect: Qt.rect(0, 0, toolBar.width,
                                        toolBar.height)
                }
            }
            Rectangle {
                anchors.fill: parent
                color: root.isThemeDark ? "darkgray" : "lightgray"
                opacity: 0.3
            }
        }

        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: qsTr("‹")
                onClicked: viewLoader.loadView(viewLoader.item.backNavigation ?? Common.mainView)
                visible: !viewLoader.isMainView()
            }

            Label {
                text: viewLoader.title()
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }

            Button {
                id: resetButton
                text: qsTr("Reset Settings")

                visible: false
                onClicked: root.resetButtonClicked()
            }

            Button {
                id: settingsButton
                text: qsTr("⚙")

                visible: false
                onClicked: root.settingsButtonClicked()
            }

            ToolButton {
                onClicked: connectionPopup.open()

                contentItem: Column {
                    id: column

                    width: Math.max(connectionLabel.implicitWidth, connStateLabel.implicitWidth) * 1.25

                    Label {
                        id: connectionLabel
                        text: qsTr("Connection")
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width
                    }

                    Rectangle {
                        color: isThemeDark ? "darkgray" : "lightgray"
                        width: parent.width
                        height: 1
                    }

                    Label {
                        id: connStateLabel
                        text: client.connected ? qsTr("CONNECTED") : qsTr("DISCONNECTED")
                        font.pointSize: connectionLabel.font.pointSize - 2
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width
                    }
                }

                Rectangle {
                    color: "transparent"

                    border.color: client.connected ? "green" : "red"
                    border.width: 1.25


                    anchors.fill: parent
                }
            }
        }
    }

    background: Loader {
        id: bgLoader

        // asynchronous: true

        readonly property bool isBlurred: status === Loader.Ready ? item.isBlurred : true

        sourceComponent: Item {

            readonly property bool isBlurred: (bgBlur.radius !== 0)

            component BgImage: Image {
                fillMode: Image.PreserveAspectCrop
            }

            Loader {
                id: bgDarkImageLoader
                active: isThemeDark
                anchors.fill: parent

                asynchronous: true

                sourceComponent: BgImage {
                    source: Common.bgDarkImage
                }
            }

            Loader {
                id: bgLightImageLoader
                active: !isThemeDark
                anchors.fill: parent

                asynchronous: true

                sourceComponent: BgImage {
                    id: bgLightImage
                    source: Common.bgLightImage
                }
            }

            FastBlur {
                id: bgBlur
                anchors.fill: parent

                radius: 64

                function blur() {
                    radius = 64
                }
                function unblur() {
                    radius = 0
                }

                Behavior on radius {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutSine
                    }
                }

                source: ShaderEffectSource {
                    sourceItem: isThemeDark ? bgDarkImageLoader.item : bgLightImageLoader.item
                }

                Component.onCompleted: {
                    unblur()
                }

                Connections {
                    target: viewLoader

                    function onBlurBackground(blur) {
                        if (blur)
                            bgBlur.blur()
                        else
                            bgBlur.unblur()
                    }
                }
            }
        }
    }

    Loader {
        id: viewLoader

        anchors.fill: parent

        source: Common.mainView

        //  asynchronous: true

        signal blurBackground(bool blur)

        onSourceChanged: {
            if ( isMainView() )
                blurBackground(false)
            else
                blurBackground(true)
        }

        function loadView(view) {
            viewLoader.source = view
        }

        function isViewLoaded(view) {
            return (Qt.resolvedUrl(source) === Qt.resolvedUrl(view))
        }

        function isMainView() {
            return isViewLoaded(Common.mainView)
        }

        function title() {
            if (viewLoader.item && viewLoader.item.title)
                return viewLoader.item.title
            else
                return qsTr("N/A")
        }

        Connections {
            target: viewLoader.item

            ignoreUnknownSignals: true

            function onViewChangeRequest(view) {
                viewLoader.loadView(view)
            }
        }

        Loader {
            active: viewLoader.item != null

            sourceComponent: Item {
                Binding {
                    when: viewLoader.item.backgroundVisible !== undefined

                    restoreMode: Binding.RestoreBindingOrValue

                    target: bgLoader
                    property: "active"
                    value: viewLoader.item.backgroundVisible
                }

                Binding {
                    when: viewLoader.item.toolbarVisible !== undefined

                    restoreMode: Binding.RestoreBindingOrValue

                    target: toolBar
                    property: "visible"
                    value: viewLoader.item.toolbarVisible
                }

                Binding {
                    when: viewLoader.item.resetButtonVisible !== undefined

                    restoreMode: Binding.RestoreBindingOrValue

                    target: resetButton
                    property: "visible"
                    value: viewLoader.item.resetButtonVisible
                }

                Binding {
                    when: viewLoader.item.settingsButtonVisible !== undefined

                    restoreMode: Binding.RestoreBindingOrValue

                    target: settingsButton
                    property: "visible"
                    value: viewLoader.item.settingsButtonVisible
                }
            }
        }
    }

    InputPanel {
        id: inputPanel
        z: 99
        x: 0
        y: root.height
        width: root.width

        // VirtualKeyboard interferes with the custom qml opengl widget !
        visible: y !== root.height

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                y: root.height - inputPanel.height
            }
        }

        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 250
                    easing.type: Easing.InOutSine
                }
            }
        }
    }
}
