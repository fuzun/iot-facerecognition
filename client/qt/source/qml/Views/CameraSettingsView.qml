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
import QtQml 2.15

import ".."
import "../Widgets" as Widgets

Widgets.SettingsList {
    readonly property string title: qsTr("Settings")
    readonly property string backNavigation: Common.cameraView
    readonly property bool resetButtonVisible: true

    model: [
        {
            model: camera.intfSettingModel,
            title: qsTr("Interface Settings")
        },
        {
            model: camera.settingModel,
            title: qsTr("Camera Settings (changes requires restart)")
        }
    ]

    Connections {
        target: root

        function onResetButtonClicked() {
            camera.settings.reset()
        }
    }

    Component.onDestruction: {
        camera.settings.save()
    }
}
