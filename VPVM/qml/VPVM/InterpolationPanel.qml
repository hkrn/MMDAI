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

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

GroupBox {
    id: interpolationPanel
    property real x1: 20
    property real y1: 20
    property real x2: 107
    property real y2: 107
    property var targetKeyframe
    property var typeModel
    property alias type: typeComboBox.currentIndex
    function updateInterpolationValues() {
        var keyframe = targetKeyframe
        if (keyframe) {
            var value = keyframe.interpolationParameter(type)
            x1 = value.x
            y1 = value.y
            x2 = value.z
            y2 = value.w
        }
    }
    Component.onCompleted: {
        scene.project.undoDidPerform.connect(updateInterpolationValues)
        scene.project.redoDidPerform.connect(updateInterpolationValues)
    }
    title: qsTr("Interpolation")
    Layout.fillHeight: true
    onX1Changed: interpolationCanvas.requestPaint()
    onX2Changed: interpolationCanvas.requestPaint()
    onY1Changed: interpolationCanvas.requestPaint()
    onY2Changed: interpolationCanvas.requestPaint()
    onTypeChanged: updateInterpolationValues()
    onTargetKeyframeChanged: updateInterpolationValues()
    RowLayout {
        Canvas {
            id: interpolationCanvas
            readonly property int minimumValue: 0
            readonly property int maximumValue: 127
            width: 128
            height: 128
            contextType: "2d"
            Path {
                id: interpolationPath
                startX: interpolationCanvas.minimumValue
                startY: interpolationCanvas.maximumValue
                PathCubic {
                    control1X: x1
                    control1Y: interpolationCanvas.maximumValue - y1
                    control2X: x2
                    control2Y: Math.abs(y2 - interpolationCanvas.maximumValue)
                    x: interpolationCanvas.maximumValue
                    y: interpolationCanvas.minimumValue
                }
            }
            onPaint: {
                context.fillStyle = Qt.rgba(1, 1, 1)
                context.fillRect(0, 0, width, height)
                context.strokeStyle = Qt.rgba(1, 0, 0)
                context.path = interpolationPath
                context.stroke()
            }
        }
        ColumnLayout {
            RowLayout {
                Label { text: qsTr("Type") }
                ComboBox {
                    id: typeComboBox
                    model: typeModel
                }
            }
            GridLayout {
                columns: 4
                Label { text: "X1" }
                SpinBox {
                    id: x1Value
                    minimumValue: interpolationCanvas.minimumValue
                    maximumValue: interpolationCanvas.maximumValue
                    value: interpolationPanel.x1
                    onValueChanged: interpolationPanel.x1 = value
                }
                Label { text: "X2" }
                SpinBox {
                    id: x2Value
                    minimumValue: interpolationCanvas.minimumValue
                    maximumValue: interpolationCanvas.maximumValue
                    value: interpolationPanel.x2
                    onValueChanged: interpolationPanel.x2 = value
                }
                Label { text: "Y1" }
                SpinBox {
                    id: y1Value
                    minimumValue: interpolationCanvas.minimumValue
                    maximumValue: interpolationCanvas.maximumValue
                    value: interpolationPanel.y1
                    onValueChanged: interpolationPanel.y1 = value
                }
                Label { text: "Y2" }
                SpinBox {
                    id: y2Value
                    minimumValue: interpolationCanvas.minimumValue
                    maximumValue: interpolationCanvas.maximumValue
                    value: interpolationPanel.y2
                    onValueChanged: interpolationPanel.y2 = value
                }
                Button {
                    Layout.columnSpan: 4
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Apply")
                    onClicked: scene.currentMotion.updateKeyframeInterpolation(targetKeyframe, Qt.vector4d(x1, y1, x2, y2), type)
                }
            }
        }
    }
}
