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
    readonly property string backNavigation: Common.statisticsView
    readonly property bool resetButtonVisible: true

    model: [
        {
            model: statistics.settingModel,
            title: qsTr("Statistics Settings")
        }
    ]

    Connections {
        target: root

        function onResetButtonClicked() {
            statistics.settings.reset()
        }
    }

    Component.onDestruction: {
        statistics.settings.save()
    }
}
