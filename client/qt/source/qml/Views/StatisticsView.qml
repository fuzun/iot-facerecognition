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

import QtCharts 2.3

import ".."

import com.iotfacerecognition.statistics 1.0
import com.iotfacerecognition.settings 1.0

Item {
    id: statisticsView

    readonly property string title: qsTr("Statistics")

    readonly property bool settingsButtonVisible: true

    signal viewChangeRequest(string view)

    Connections {
        target: root

        function onSettingsButtonClicked() {
            viewChangeRequest(Common.statisticsSettingsView)
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent

        contentWidth: width;
        contentHeight: column.height;

        ScrollBar.vertical: ScrollBar {
            visible: flickable.contentHeight > flickable.height
            policy: ScrollBar.AlwaysOn
        }

        Column {
            id: column

            anchors.left: parent.left
            anchors.right: parent.right

            component CustomChartView : ChartView {
                theme: intf.settings.darkMode ? ChartView.ChartThemeDark
                                              : ChartView.ChartThemeLight
                opacity: 0.85

                implicitHeight: 350

                property bool showAllOnPressed: true

                antialiasing: statistics.settings.useAntialiasing

                animationDuration: 50
                animationEasingCurve.type: Easing.InOutSine
                animationOptions: statistics.settings.enableAnimations ? ChartView.SeriesAnimations
                                                                       : ChartView.NoAnimation

                signal pressed()
                signal released()

                Loader {
                    anchors.fill: parent
                    active: showAllOnPressed

                    sourceComponent: MouseArea {
                        propagateComposedEvents: true

                        onPressed: {
                            parent.parent.pressed()
                        }

                        onReleased: {
                            parent.parent.released()
                        }
                    }
                }
            }

            component CustomLineSeries : LineSeries {
                property point max: Qt.point(Number.MIN_VALUE, Number.MIN_VALUE)
                property point min: Qt.point(Number.MAX_VALUE, Number.MAX_VALUE)

                required property var parent
                required property int type

                property var averageSeries: undefined

                pointsVisible: true

                function adjustAxisX() {
                    if (axisX instanceof ValueAxis) {
                        axisX.max = Qt.binding( () => { return max.x })
                        axisX.min = Qt.binding( () => { return max.x - statistics.settings.stdWidthXValueAxis })
                    } else if (axisX instanceof DateTimeAxis) {
                        axisX.max = Qt.binding( () => { return new Date(max.x) })
                        axisX.min = Qt.binding( () => { return new Date(max.x - (statistics.settings.stdWidthXDateTimeAxis * 1000)) })
                    }
                }

                function adjustAxisY() {
                    axisY.max = Qt.binding( () => { return max.y + (Math.abs(max.y) * 0.5) })
                    axisY.min = Qt.binding( () => { return min.y - (Math.abs(min.y) * 0.5) })
                }

                Component.onCompleted: {
                    statistics.attachSeries(this);

                    adjustAxisX()
                    adjustAxisY()
                }

                Component.onDestruction: {
                    statistics.detachSeries(this)
                }

                Connections {
                    target: parent

                    function onPressed() {
                        if (axisX instanceof ValueAxis) {
                            axisX.max = Qt.binding( () => { return max.x })
                            axisX.min = Qt.binding( () => { return min.x })
                        } else if (axisX instanceof DateTimeAxis) {
                            axisX.max = Qt.binding( () => { return new Date(max.x) })
                            axisX.min = Qt.binding( () => { return new Date(min.x) })
                        }
                    }

                    function onReleased() {
                        adjustAxisX()
                    }
                }
            }

            component BasePieSeries : PieSeries {

                required property int type

                Component.onCompleted: {
                    statistics.attachSeries(this)
                }

                Component.onDestruction: {
                    statistics.detachSeries(this)
                }
            }

            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right

                Loader {
                    active: statistics.settings.loadRecognizedMaxPredictionObjectsPieSeries

                    asynchronous: true

                    Layout.fillWidth: status === Loader.Ready ? true : false

                    sourceComponent: CustomChartView {
                        title: qsTr("Recognized Objects with Maximum Prediction")

                        BasePieSeries {
                            type: Statistics.RecognizedMaxPredictionObjectsPieSeries
                        }
                    }
                }

                Loader {
                    active: statistics.settings.loadRecognizedFacesPieSeries

                    asynchronous: true

                    Layout.fillWidth: status === Loader.Ready ? true : false

                    sourceComponent: CustomChartView {
                        title: qsTr("Recognized Faces")

                        BasePieSeries {
                            type: Statistics.RecognizedFacesPieSeries
                        }
                    }
                }
            }

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: statistics.settings.loadRecognizedFacesLineSeries

                asynchronous: true

                sourceComponent: CustomChartView {
                    id: foundFacesChart

                    DateTimeAxis { id: faceSeriesXAxis; format: "hh:mm:ss" }

                    LineSeries {
                        id: untaggedFacesAverage

                        axisX: faceSeriesXAxis
                        axisY: untaggedFaces.axisY

                        pointsVisible: true
                    }

                    CustomLineSeries {
                        id: untaggedFaces

                        type: Statistics.UntaggedFacesLineSeries
                        name: qsTr("Recognized Untagged Faces")

                        averageSeries: untaggedFacesAverage

                        parent: foundFacesChart

                        axisX: faceSeriesXAxis
                        axisY: ValueAxis { titleText: qsTr("Untagged Count") }
                    }

                    LineSeries {
                        id: taggedFacesAverage

                        axisX: faceSeriesXAxis
                        axisY: taggedFaces.axisY

                        pointsVisible: true
                    }

                    CustomLineSeries {
                        id: taggedFaces

                        type: Statistics.TaggedFacesLineSeries
                        name: qsTr("Recognized Tagged Faces")

                        averageSeries: taggedFacesAverage

                        parent: foundFacesChart

                        axisX: faceSeriesXAxis
                        axisY: ValueAxis { titleText: qsTr("Tagged Count") }
                    }
                }
            }

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: statistics.settings.loadMaxObjectPredictionRateLineSeries

                asynchronous: true

                sourceComponent: CustomChartView {
                    id: objectPredictionChart

                    ValueAxis { id: objectPredictionYAxis; titleText: qsTr("Count") }
                    DateTimeAxis { id: objectPredictionXAxis; format: "hh:mm:ss" }

                    LineSeries {
                        id: maxObjectPredictionRateAverage

                        axisX: objectPredictionXAxis
                        axisY: objectPredictionYAxis

                        pointsVisible: true
                    }

                    CustomLineSeries {
                        name: qsTr("Maximum Object Prediction Rate")

                        type: Statistics.MaximumObjectPredictionRateLineSeries

                        averageSeries: maxObjectPredictionRateAverage

                        parent: objectPredictionChart

                        axisX: objectPredictionXAxis
                        axisY: objectPredictionYAxis
                    }
                }
            }

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: statistics.settings.loadInternetUsageCumulativeChart

                asynchronous: true

                sourceComponent: CustomChartView {
                    id: internetUsageCumChart
                    title: qsTr("Internet Usage (Cumulative)")

                    DateTimeAxis { id: internetUsageCumXAxis; format: "hh:mm:ss" }

                    CustomLineSeries {
                        name: qsTr("Sent (MBytes)")

                        type: Statistics.InternetUsageSentCumulativeLineSeries
                        parent: internetUsageCumChart

                        axisX: internetUsageCumXAxis
                        axisY: ValueAxis { titleText: qsTr("MBytes") }
                    }

                    CustomLineSeries {
                        name: qsTr("Received (Bytes)")

                        type: Statistics.InternetUsageReceivedCumulativeLineSeries
                        parent: internetUsageCumChart

                        axisX: internetUsageCumXAxis
                        axisY: ValueAxis { titleText: qsTr("Bytes") }
                    }
                }
            }

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: statistics.settings.loadInternetUsageChart

                asynchronous: true

                sourceComponent: CustomChartView {
                    id: internetUsageChart
                    title: qsTr("Internet Usage")

                    ValueAxis { id: internetUsageXAxis; titleText: qsTr("Elapsed Time (seconds)") }

                    BarSeries {
                        BarSet {
                            id: averageSent

                            borderColor: "transparent"
                        }

                        axisY: sentLineSeries.axisY
                        axisX: internetUsageXAxis
                    }

                    BarSeries {
                        BarSet {
                            id: averageReceived

                            borderColor: "transparent"
                        }

                        axisY: receivedLineSeries.axisY
                        axisX: internetUsageXAxis
                    }

                    CustomLineSeries {
                        id: sentLineSeries

                        type: Statistics.InternetUsageSentLineSeries
                        name: qsTr("Sent (KBytes/s)")

                        averageSeries: averageSent

                        parent: internetUsageChart

                        axisX: internetUsageXAxis
                        axisY: ValueAxis { titleText: qsTr("KBytes/s") }
                    }

                    CustomLineSeries {
                        id: receivedLineSeries

                        type: Statistics.InternetUsageReceivedLineSeries
                        name: qsTr("Received (Bytes/s)")

                        parent: internetUsageChart

                        averageSeries: averageReceived

                        axisX: internetUsageXAxis
                        axisY: ValueAxis { titleText: qsTr("Bytes/s")  }
                    }
                }
            }

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: statistics.settings.loadFPSChart

                asynchronous: true

                sourceComponent: CustomChartView {
                    id: fpsChart

                    BarSeries {
                        name: qsTr("Average FPS")

                        BarSet {
                            id: averageFPS
                            borderColor: "transparent"
                        }

                        axisY: fpsLineSeries.axisY
                        axisX: fpsLineSeries.axisX
                    }

                    CustomLineSeries {
                        id: fpsLineSeries
                        name: qsTr("FPS")

                        type: Statistics.FPSLineSeries
                        parent: fpsChart

                        averageSeries: averageFPS

                        axisY: ValueAxis { titleText: qsTr("Frames per Second") }
                        axisX: ValueAxis { titleText: qsTr("Elapsed Time (seconds)") }
                    }
                }
            }
        }
    }
}
