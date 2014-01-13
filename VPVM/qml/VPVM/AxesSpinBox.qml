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
    id: axesSpinBox
    property real maximumValue
    property real minimumValue
    property real stepSize
    property int decimals: 0
    property string labelX: "X"
    property string labelY: "Y"
    property string labelZ: "Z"
    property bool activeFocusOnSpinBox: false
    property bool hovered: false
    property bool resettable: false
    property var value
    signal resetDidTrigger()
    signal editingFinished()
    Component.onCompleted: {
        spinboxX.editingFinished.connect(editingFinished)
        spinboxY.editingFinished.connect(editingFinished)
        spinboxZ.editingFinished.connect(editingFinished)
    }
    onResetDidTrigger: value.x = value.y = value.z = spinboxX.value = spinboxY.value = spinboxZ.value = 0
    onValueChanged: {
        spinboxX.value = value.x
        spinboxY.value = value.y
        spinboxZ.value = value.z
    }
    GridLayout {
        columns: 2
        Label { text: axesSpinBox.labelX }
        SpinBox {
            id: spinboxX
            maximumValue: axesSpinBox.maximumValue
            minimumValue: axesSpinBox.minimumValue
            decimals: axesSpinBox.decimals
            stepSize: axesSpinBox.stepSize
            value: axesSpinBox.value ? axesSpinBox.value.x : 0
            onHoveredChanged: axesSpinBox.hovered = hovered
            onValueChanged: if (hovered) axesSpinBox.value.x = value
            onActiveFocusChanged: {
                axesSpinBox.activeFocusOnSpinBox = activeFocus
                if (!activeFocus) {
                    axesSpinBox.value.x = value
                }
            }
        }
        Label { text: axesSpinBox.labelY }
        SpinBox {
            id: spinboxY
            maximumValue: axesSpinBox.maximumValue
            minimumValue: axesSpinBox.minimumValue
            decimals: axesSpinBox.decimals
            stepSize: axesSpinBox.stepSize
            value: axesSpinBox.value ? axesSpinBox.value.y : 0
            onHoveredChanged: axesSpinBox.hovered = hovered
            onValueChanged: if (hovered) axesSpinBox.value.y = value
            onActiveFocusChanged: {
                axesSpinBox.activeFocusOnSpinBox = activeFocus
                if (!activeFocus) {
                    axesSpinBox.value.y = value
                }
            }
        }
        Label { text: axesSpinBox.labelZ }
        SpinBox {
            id: spinboxZ
            maximumValue: axesSpinBox.maximumValue
            minimumValue: axesSpinBox.minimumValue
            decimals: axesSpinBox.decimals
            stepSize: axesSpinBox.stepSize
            value: axesSpinBox.value ? axesSpinBox.value.z : 0
            onHoveredChanged: axesSpinBox.hovered = hovered
            onValueChanged: if (hovered) axesSpinBox.value.z = value
            onActiveFocusChanged: {
                axesSpinBox.activeFocusOnSpinBox = activeFocus
                if (!activeFocus) {
                    axesSpinBox.value.z = value
                }
            }
        }
        Button {
            visible: axesSpinBox.resettable
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Reset")
            onClicked: resetDidTrigger()
        }
    }
}
