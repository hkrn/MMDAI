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
    id: exportTab
    title: qsTr("Export")
    anchors.margins: propertyPanel.anchors.margins
    property bool isGridIncluded : false
    property size size
    property size range : Qt.size(0, 0)
    property string videoType
    property string frameImageType
    RowLayout {
        GroupBox {
            Layout.fillHeight: true
            title: qsTr("Image")
            ColumnLayout {
                CheckBox {
                    id: enablePreset
                    text: qsTr("Use Preset")
                    checked: true
                }
                ListModel {
                    id: presetExportSizeModel
                    ListElement { text: "360p (640x360)"; width: 640; height: 360 }
                    ListElement { text: "480p (854x480)"; width: 854; height: 480 }
                    ListElement { text: "HD 720p (1280x720)"; width: 1280; height: 720 }
                    ListElement { text: "HD 1080p (1920x1080)"; width: 1920; height: 1080 }
                }
                ComboBox {
                    Layout.alignment: Qt.AlignCenter
                    visible: enablePreset.checked
                    model: presetExportSizeModel
                    onCurrentIndexChanged: {
                        var item = presetExportSizeModel.get(currentIndex)
                        exportTab.size = Qt.size(item.width, item.height)
                        imageWidthSpinbox.value = item.width
                        imageHeightSpinbox.value = item.height
                    }
                }
                GridLayout {
                    columns: 2
                    visible: !enablePreset.checked
                    Layout.alignment: Qt.AlignCenter
                    Label { text: qsTr("Width") }
                    SpinBox {
                        id: imageWidthSpinbox
                        minimumValue: 1
                        maximumValue: 8192
                        suffix: "px"
                        value: scene.viewport.width
                        onValueChanged: size.width = value
                    }
                    Label { text: qsTr("Height") }
                    SpinBox {
                        id: imageHeightSpinbox
                        minimumValue: 1
                        maximumValue: 8192
                        suffix: "px"
                        value: scene.viewport.height
                        onValueChanged: size.height = value
                    }
                }
                Button {
                    Layout.columnSpan: 2
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Export")
                    action: exportImageAction
                }
            }
        }
        GroupBox {
            Layout.fillHeight: true
            title: qsTr("Video (shares Image width/height setting)")
            enabled: playSceneAction.enabled
            ColumnLayout {
                RowLayout {
                    GroupBox {
                        id: encodingSetting
                        title: qsTr("Custom Encode Settings")
                        checkable: true
                        checked: false
                        Layout.fillHeight: true
                        GridLayout {
                            enabled: encodingSetting.checked
                            columns: 2
                            Label { text: qsTr("Video Type") }
                            ListModel {
                                id: videoTypeModel
                                ListElement { text: "PNG"; value: "png:rgb24" }
                                ListElement { text: "UtVideo RGBA"; value: "utvideo:rgba"  }
                                ListElement { text: "UtVideo YUV422"; value: "utvideo:yuv422p" }
                                ListElement { text: "UtVideo YUV420"; value: "utvideo:yuv420p" }
                            }
                            ListModel {
                                id: frameImageTypeModel
                                ListElement { text: "BMP"; value: "bmp" }
                                ListElement { text: "PNG"; value: "png" }
                            }
                            ComboBox {
                                model: videoTypeModel
                                currentIndex: 0
                                onCurrentIndexChanged: videoType = videoTypeModel.get(currentIndex).value
                            }
                            Label { text: qsTr("Frame Image Type") }
                            ComboBox {
                                model: frameImageTypeModel
                                currentIndex: 0
                                onCurrentIndexChanged: frameImageType = frameImageTypeModel.get(currentIndex).value
                            }
                        }
                    }
                    GroupBox {
                        id: outputRange
                        title: qsTr("Custom Output Range")
                        checkable: true
                        checked: false
                        Layout.fillHeight: true
                        GridLayout {
                            enabled: outputRange.checked
                            columns: 2
                            Label { text: qsTr("From") }
                            SpinBox {
                                id: outputRangeFrom
                                minimumValue: 0
                                maximumValue: Math.pow(2, 31)
                                value: 0
                                onValueChanged: range.width = value
                            }
                            Label { text: qsTr("To") }
                            SpinBox {
                                id: outputRangeTo
                                function __handleDurationTimeIndexChanged() {
                                    value = scene.project.durationTimeIndex
                                }
                                Component.onCompleted: {
                                    scene.project.durationTimeIndexChanged.connect(__handleDurationTimeIndexChanged)
                                }
                                minimumValue: outputRangeFrom.value
                                maximumValue: outputRangeFrom.maximumValue
                                value: scene.project.durationTimeIndex
                                onValueChanged: range.height = value
                            }
                        }
                    }
                    CheckBox {
                        text: qsTr("Include Grid")
                        checked: true
                        onCheckedChanged: isGridIncluded = value
                    }
                }
                Button {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Export")
                    action: exportVideoAction
                }
            }
        }
        Item { Layout.fillWidth: true }
    }
}
