/**

 Copyright (c) 2010-2014  hkrn

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
import com.github.mmdai.VPMM 1.0 as VPMM

ScrollView {
    id: ikView
    property var targetObject
    Item {
        id: ikContentView
        VPMM.Vector3 { id: upperLimit; value: targetObject.upperLimit }
        VPMM.Vector3 { id: lowerLimit; value: targetObject.lowerLimit }
        Binding {
            target: targetObject
            property: "upperLimit"
            value: upperLimit.value
            when: ikLowerLimitXSpinBox.hovered || ikLowerLimitYSpinBox.hovered || ikLowerLimitZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "lowerLimit"
            value: lowerLimit.value
            when: ikUpperLimitXSpinBox.hovered || ikUpperLimitYSpinBox.hovered || ikUpperLimitZSpinBox.hovered
        }
        GridLayout {
            columns: 2
            Label { text: qsTr("Target Bone") }
            ComboBox {
                model: bonesModel
                editable: true
                Layout.fillWidth: true
            }
            Label { text: qsTr("Number of Loops") }
            SpinBox {
                maximumValue: 255
                minimumValue: 0
            }
            Label { text: qsTr("Angle") }
            SpinBox {
                maximumValue: 360
                minimumValue: -maximumValue
            }
            RowLayout {
                Layout.columnSpan: 2
                GroupBox {
                    title: qsTr("Lower Angle Limit")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: ikLowerLimitXSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: lowerLimit.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: ikLowerLimitYSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: lowerLimit.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: ikLowerLimitZSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: lowerLimit.z
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Upper Angle Limit")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: ikUpperLimitXSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: upperLimit.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: ikUpperLimitYSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: upperLimit.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: ikUpperLimitZSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: upperLimit.z
                        }
                    }
                }
            }
            TableView {
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }
            RowLayout {
                Layout.columnSpan: 2
                Label { text: qsTr("Link Bone") }
                ComboBox {
                    model: bonesModel
                    editable: true
                }
                Button { text: qsTr("Add") }
            }
        }
        Item { height: 20 }
    }
}
