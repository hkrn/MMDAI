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
    id: boneView
    property var targetObject
    Item {
        id: boneContentView
        VPMM.Vector3 { id: boneOrigin; value: targetObject.origin }
        VPMM.Vector3 { id: boneDestination; value: targetObject.destinationOrigin }
        VPMM.Vector3 { id: boneFixedAxis; value: targetObject.fixedAxis }
        Binding {
            target: targetObject
            property: "origin"
            value: boneOrigin.value
            when: boneOriginXSpinBox.hovered || boneOriginYSpinBox.hovered || boneOriginZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "destinationOrigin"
            value: boneDestination.value
            when: boneDestinationXSoinBox.hovered || boneDestinationYSoinBox.hovered || boneDestinationZSoinBox.hovered
        }
        Binding {
            target: targetObject
            property: "fixedAixs"
            value: boneFixedAxis.value
            when: boneDestinationXSoinBox.hovered || boneDestinationYSoinBox.hovered || boneDestinationZSoinBox.hovered
        }
        ColumnLayout {
            id: boneContentViewLayout
            anchors.fill: parent
            anchors.margins: 12
            Component.onCompleted: boneContentView.height = childrenRect.height
            RowLayout {
                Label { text: qsTr("Name") }
                TextField {
                    id: boneNameTextField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Input Bone Name Here")
                    text: targetObject.name
                }
                Binding {
                    target: targetObject
                    property: "name"
                    value: boneNameTextField.text
                    when: boneNameTextField.hovered
                }
            }
            RowLayout {
                ExclusiveGroup { id: destinationTypeGroup }
                Label { text: qsTr("Destination Type") }
                RadioButton {
                    id: destinationTypeBone
                    text: qsTr("Bone")
                    checked: targetObject.destinationOriginBone !== null
                    exclusiveGroup: destinationTypeGroup
                }
                RadioButton {
                    id: destinationTypePosition
                    text: qsTr("Position")
                    checked: targetObject.destinationOriginBone === null
                    exclusiveGroup: destinationTypeGroup
                }
            }
            RowLayout {
                enabled: destinationTypeBone.checked
                Label { text: qsTr("Destination Bone") }
                ComboBox {
                    id: boneDestinationOriginBoneComboBox
                    model: bonesModel
                    editable: true
                    Layout.fillWidth: true
                    currentIndex: bonesModel.indexOf(targetObject.destinationOriginBone)
                }
                Binding {
                    target: targetObject
                    property: "destinationOriginBone"
                    value: bonesModel.get(boneDestinationOriginBoneComboBox.currentIndex).item
                    when: boneDestinationOriginBoneComboBox.hovered
                }
            }
            RowLayout {
                GroupBox {
                    title: qsTr("Origin")
                    Layout.fillWidth: true
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: boneOriginXSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: boneOrigin.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: boneOriginYSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: boneOrigin.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: boneOriginZSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: boneOrigin.z
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Destination")
                    enabled: destinationTypePosition.checked
                    Layout.fillWidth: true
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: boneDestinationXSoinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: boneDestination.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: boneDestinationYSoinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: boneDestination.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: boneDestinationZSoinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: boneDestination.z
                        }
                    }
                }
            }
            GroupBox {
                title: qsTr("Capabilities")
                Layout.fillWidth: true
                ColumnLayout {
                    CheckBox {
                        id: boneRotateableCheckbox
                        text: qsTr("Rotateable")
                        checked: targetObject.rotateable
                    }
                    Binding {
                        target: targetObject
                        property: "rotateable"
                        value: boneRotateableCheckbox.checked
                    }
                    CheckBox {
                        id: boneMovableCheckbox
                        text: qsTr("Movable")
                        checked: targetObject.movable
                    }
                    Binding {
                        target: targetObject
                        property: "movable"
                        value: boneMovableCheckbox.checked
                    }
                    CheckBox {
                        id: boneVisibleCheckbox
                        text: qsTr("Visible")
                        checked: targetObject.visible
                    }
                    Binding {
                        target: targetObject
                        property: "visible"
                        value: boneVisibleCheckbox.checked
                    }
                    CheckBox {
                        id: boneInteractiveCheckbox
                        text: qsTr("Interactive")
                        checked: targetObject.interactive
                    }
                    Binding {
                        target: targetObject
                        property: "interactive"
                        value: boneInteractiveCheckbox.checked
                    }
                    CheckBox {
                        id: boneInherentOrientationCheckbox
                        text: qsTr("Inherent Orientation")
                        checked: targetObject.inherentOrientationEnabled
                    }
                    Binding {
                        target: targetObject
                        property: "inherentOrientation"
                        value: boneInherentOrientationCheckbox.checked
                    }
                    CheckBox {
                        id: boneInherentTranslationCheckbox
                        text: qsTr("Inherent Translation")
                        checked: targetObject.inherentTranslationEnabled
                    }
                    Binding {
                        target: targetObject
                        property: "inherentTranslation"
                        value: boneInherentTranslationCheckbox.checked
                    }
                }
            }
            GroupBox {
                title: qsTr("Inherent")
                enabled: boneInherentTranslationCheckbox.checked || boneInherentOrientationCheckbox.checked
                Layout.fillWidth: true
                GridLayout {
                    columns: 2
                    Label { text: qsTr("Bone") }
                    ComboBox {
                        id: boneInherentBoneComboBox
                        model: bonesModel
                        editable: true
                        Layout.fillWidth: true
                        currentIndex: bonesModel.indexOf(targetObject.inherentBone)
                    }
                    Binding {
                        target: targetObject
                        property: "inherentBone"
                        value: bonesModel.get(boneInherentBoneComboBox.currentIndex).item
                        when: boneInherentBoneComboBox.hovered
                    }
                    Label { text: qsTr("Coefficient") }
                    SpinBox {
                        id: boneInherentCoefficientSpinBox
                        maximumValue: 100
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.inherentCoefficient
                    }
                    Binding {
                        target: targetObject
                        property: "inherentCoefficient"
                        value: boneInherentCoefficientSpinBox.value
                        when: boneInherentCoefficientSpinBox.hovered
                    }
                }
            }
            GroupBox {
                id: boneLocalAxesGroupBox
                title: qsTr("Local Axes")
                checkable: true
                checked: targetObject.localAxesEnabled
                Layout.fillWidth: true
                GridLayout {
                    columns: 2
                    Label { text: "X" }
                    SpinBox {
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                    }
                    Label { text: "Y" }
                    SpinBox {
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                    }
                    Label { text: "Z" }
                    SpinBox {
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                    }
                }
            }
            Binding {
                target: targetObject
                property: "localAxes"
                value: boneLocalAxesGroupBox.checked
            }
            GroupBox {
                id: boneFixedAxisGroupBox
                title: qsTr("Fixed Axes")
                checkable: true
                checked: targetObject.fixedAxisEnabled
                Layout.fillWidth: true
                RowLayout {
                    GroupBox {
                        title: qsTr("X Axis")
                        GridLayout {
                            columns: 2
                            Label { text: "X" }
                            SpinBox {
                                id: boneFixedAixsXSpinBox
                                maximumValue: 1
                                minimumValue: -maximumValue
                                decimals: 5
                                stepSize: 0.01
                                value: boneFixedAxis.x
                            }
                            Label { text: "Y" }
                            SpinBox {
                                id: boneFixedAixsYSpinBox
                                maximumValue: 1
                                minimumValue: -maximumValue
                                decimals: 5
                                stepSize: 0.01
                                value: boneFixedAxis.y
                            }
                            Label { text: "Z" }
                            SpinBox {
                                id: boneFixedAixsZSpinBox
                                maximumValue: 1
                                minimumValue: -maximumValue
                                decimals: 5
                                stepSize: 0.01
                                value: boneFixedAxis.z
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Z Axis")
                        GridLayout {
                            columns: 2
                            Label { text: "X" }
                            SpinBox {
                                maximumValue: 1
                                minimumValue: -maximumValue
                                decimals: 5
                                stepSize: 0.01
                            }
                            Label { text: "Y" }
                            SpinBox {
                                maximumValue: 1
                                minimumValue: -maximumValue
                                decimals: 5
                                stepSize: 0.01
                            }
                            Label { text: "Z" }
                            SpinBox {
                                maximumValue: 1
                                minimumValue: -maximumValue
                                decimals: 5
                                stepSize: 0.01
                            }
                        }
                    }
                }
            }
            Binding {
                target: targetObject
                property: "fixedAxes"
                value: boneFixedAxisGroupBox.checked
            }
            GroupBox {
                id: boneIKGroupBox
                title: qsTr("IK (Inverse Kinematics)")
                checkable: true
                checked: targetObject.inverseKinematicsEnabled
                Layout.fillWidth: true
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
                    RowLayout {
                        Layout.columnSpan: 2
                        enabled: false
                        GroupBox {
                            title: qsTr("Lower Angle Limit")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("Upper Angle Limit")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                }
                            }
                        }
                    }
                }
            }
            Binding {
                target: targetObject
                property: "hasInversedKinematics"
                value: boneIKGroupBox.checked
            }
            Item { height: 20 }
        }
    }
}
