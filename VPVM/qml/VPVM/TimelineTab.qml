/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Tab {
    id: timelineTab
    title: qsTr("Timeline")
    anchors.margins: propertyPanel.anchors.margins
    RowLayout {
        GroupBox {
            Layout.fillHeight: true
            title: qsTr("Duration/Position Setting")
            GridLayout {
                id: setting
                columns: 2
                Label { text: qsTr("Estimated Duration") }
                SpinBox {
                    id: estimatedDurationInIndex
                    minimumValue: 900
                    maximumValue: Math.pow(2, 31)
                    value: timeline.durationTimeIndex
                    onValueChanged: timeline.durationTimeIndex = value
                }
                TextField {
                    id: estimatedDurationInSeconds
                    function padding(value) {
                        return value < 10 ? ("0%1".arg(value)) : value
                    }
                    function format(seconds) {
                        var sec = seconds % 60,
                                min = Math.floor(seconds / 60) % 60,
                                hour = Math.floor(seconds / 3600)
                        return "%1:%2:%3".arg(padding(hour)).arg(padding(min)).arg(padding(sec))
                    }
                    function toSeconds(value) {
                        var tokens = value.split(":"),
                                hour = tokens[0],
                                min = tokens[1],
                                sec = tokens[2],
                                result = parseInt(sec, 10) + parseInt(min * 60, 10) + parseInt(hour * 3600, 10)
                        return result
                    }
                    inputMethodHints: Qt.ImhDate
                    text: format(timeline.durationSeconds)
                    validator: RegExpValidator { regExp: /^[0-9][0-9]:[0-5][0-9]:[0-5][0-9]$/ }
                    onAccepted: timeline.durationSeconds = toSeconds(text)
                }
                Label { text: qsTr("Current Position") }
                SpinBox {
                    id: currentPositionInIndex
                    minimumValue: 0
                    maximumValue: timeline.durationTimeIndex
                    value: timeline.timeIndex
                    onValueChanged: timeline.timeIndex = value
                }
                TextField {
                    id: currentPositionInSeconds
                    inputMethodHints: estimatedDurationInSeconds.inputMethodHints
                    text: estimatedDurationInSeconds.format(timeline.timeSeconds)
                    validator: estimatedDurationInSeconds.validator
                    onAccepted: timeline.timeSeconds = estimatedDurationInSeconds.toSeconds(text)
                }
                Label { text: qsTr("Scale Factor") }
                SpinBox {
                    minimumValue: timeline.minimumTimeScaleFactor
                    maximumValue: timeline.maximumTimeScaleFactor
                    decimals: 3
                    stepSize: 0.005
                    value: timeline.timeScaleFactor
                    onValueChanged: {
                        if (hovered) {
                            timeline.timeScaleFactor = value
                            timeline.refresh()
                        }
                    }
                }
                CheckBox {
                    Layout.columnSpan: 2
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Show in Time")
                    onCheckedChanged: parent.state = checked ? "timeSeconds" : "timeIndex"
                }
                state: "timeIndex"
                states: [
                    State {
                        name: "timeIndex"
                        PropertyChanges { target: estimatedDurationInSeconds; visible: false }
                        PropertyChanges { target: currentPositionInSeconds; visible: false }
                        PropertyChanges { target: estimatedDurationInIndex; visible: true }
                        PropertyChanges { target: currentPositionInIndex; visible: true }
                    },
                    State {
                        name: "timeSeconds"
                        PropertyChanges { target: estimatedDurationInSeconds; visible: true }
                        PropertyChanges { target: currentPositionInSeconds; visible: true }
                        PropertyChanges { target: estimatedDurationInIndex; visible: false }
                        PropertyChanges { target: currentPositionInIndex; visible: false }
                    }
                ]
            }
        }
        GroupBox {
            Layout.fillHeight: true
            title: qsTr("Ranged Selection")
            GridLayout {
                columns: 2
                Label { text: qsTr("From") }
                SpinBox {
                    id: selectRangeFrom
                    minimumValue: 0
                    maximumValue: estimatedDurationInIndex.maximumValue
                    value: 0
                }
                Label { text: qsTr("To") }
                SpinBox {
                    id: selectRangeTo
                    minimumValue: selectRangeFrom.value
                    maximumValue: estimatedDurationInIndex.maximumValue
                    value: 0
                }
                CheckBox {
                    id: selectVisibleTracksOnly
                    Layout.columnSpan: 2
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Visible Tracks Only")
                    checked: true
                }
                Button {
                    Layout.columnSpan: 2
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Select")
                    onClicked: timeline.selectRange(selectRangeFrom.value, selectRangeTo.value, selectVisibleTracksOnly.checked)
                }
            }
        }
        GroupBox {
            id: rangedPlaying
            Layout.fillHeight: true
            title: qsTr("Ranged Playing")
            checkable: true
            checked: false
            function updateRange() {
                if (playRangeFrom.value >= 0 && playRangeTo.value > 0) {
                    var to = Math.min(playRangeTo.value, scene.project.durationTimeIndex)
                    scene.setRange(playRangeFrom.value, to)
                }
            }
            onCheckedChanged: {
                if (checked) {
                    updateRange()
                }
                else if (scene.project.durationTimeIndex > 0) {
                    scene.setRange(0, scene.project.durationTimeIndex)
                }
            }
            GridLayout {
                columns: 2
                Label { text: qsTr("From") }
                SpinBox {
                    id: playRangeFrom
                    minimumValue: 0
                    maximumValue: estimatedDurationInIndex.maximumValue
                    value: 0
                    onValueChanged: rangedPlaying.updateRange()
                }
                Label { text: qsTr("To") }
                SpinBox {
                    id: playRangeTo
                    minimumValue: playRangeFrom.value
                    maximumValue: estimatedDurationInIndex.maximumValue
                    value: scene.project.durationTimeIndex
                    onValueChanged: rangedPlaying.updateRange()
                }
                CheckBox {
                    Layout.alignment: Qt.AlignCenter
                    Layout.columnSpan: 2
                    text: playLoopAction.text
                    checked: playLoopAction.checked
                    onCheckedChanged: playLoopAction.checked = checked
                }
            }
        }
        Rectangle { Layout.fillWidth: true }
    }
}
