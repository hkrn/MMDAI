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
    property int videoType : 0
    property int frameImageType : 0
    RowLayout {
        GroupBox {
            Layout.fillHeight: true
            title: qsTr("Image")
            GridLayout {
                columns: 2
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
            ColumnLayout {
                RowLayout {
                    GroupBox {
                        id: encodingSetting
                        title: qsTr("Encoding Setting")
                        checkable: true
                        checked: false
                        Layout.fillHeight: true
                        GridLayout {
                            enabled: encodingSetting.checked
                            columns: 2
                            Label { text: qsTr("Video Type") }
                            ComboBox {
                                model: [
                                    qsTr("UtVideo RGBA"),
                                    qsTr("UtVideo YUV422"),
                                    qsTr("UtVideo YUV420"),
                                    qsTr("PNG")
                                ]
                                onCurrentIndexChanged: videoType = currentIndex
                            }
                            Label { text: qsTr("Frame Image Type") }
                            ComboBox {
                                model: [
                                    qsTr("bmp"),
                                    qsTr("png")
                                ]
                                onCurrentIndexChanged: frameImageType = currentIndex
                            }
                        }
                    }
                    GroupBox {
                        id: outputRange
                        title: qsTr("Output Range")
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
                                maximumValue: timeline.durationTimeIndex
                                value: 0
                                onValueChanged: range.width = value
                            }
                            Label { text: qsTr("To") }
                            SpinBox {
                                id: outputRangeTo
                                minimumValue: outputRangeFrom.value + 1
                                maximumValue: timeline.durationTimeIndex
                                value: timeline.durationTimeIndex
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
        Rectangle { Layout.fillWidth: true }
    }
}
