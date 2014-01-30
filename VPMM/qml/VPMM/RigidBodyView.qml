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
    id: rigidBodyView
    property var targetObject
    Item {
        id: rigidBodyContentView
        VPMM.Vector3 { id: rigidBodyPosition; value: targetObject.position }
        VPMM.Vector3 { id: rigidBodyRotation; value: targetObject.rotation }
        Binding {
            target: targetObject
            property: "posiiton"
            value: rigidBodyPosition.value
            when: rigidBodyPositionXSpinBox.hovered || rigidBodyPositionYSpinBox.hovered || rigidBodyPositionZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "rotation"
            value: rigidBodyRotation.value
            when: rigidBodyRotationXSpinBox.hovered || rigidBodyRotationXSpinBox.hovered || rigidBodyRotationXSpinBox.hovered
        }
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            Component.onCompleted: rigidBodyContentView.height = childrenRect.height
            GridLayout {
                columns: 2
                Label { text: qsTr("Name") }
                TextField {
                    id: rigidBodyNameTextField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Input Rigid Body Name Here")
                    text: targetObject.name
                }
                Binding {
                    target: targetObject
                    property: "name"
                    value: rigidBodyNameTextField.text
                    when: rigidBodyNameTextField.hovered
                }
                Label { text: qsTr("Bone") }
                ComboBox {
                    id: rigidBodyBoneComboBox
                    Layout.fillWidth: true
                    model: bonesModel
                    editable: true
                    currentIndex: bonesModel.indexOf(targetObject.parentBone)
                }
                Binding {
                    target: targetObject
                    property: "bone"
                    value: targetObject.parentModel.allBones[rigidBodyBoneComboBox.currentIndex]
                }
                Label { text: qsTr("Type") }
                ComboBox {
                    id: objectTypeComboBox
                    function indexOf(value) {
                        var result = model.filter(function(element){ return element.value === value })
                        return result.length > 0 ? result[0].value : -1
                    }
                    model: [
                        { "text": qsTr("Static"),  "value": VPMM.RigidBody.StaticObject },
                        { "text": qsTr("Dynamic"), "value": VPMM.RigidBody.DynamicObject },
                        { "text": qsTr("Aligned"), "value": VPMM.RigidBody.AlignedObject }
                    ]
                    currentIndex: indexOf(targetObject.objectType)
                }
                Binding {
                    target: targetObject
                    property: "objectType"
                    value: objectTypeComboBox.model[objectTypeComboBox.currentIndex].value
                    when: objectTypeComboBox.hovered
                }
            }
            GroupBox {
                title: qsTr("Shape")
                Layout.fillWidth: true
                GridLayout {
                    columns: 2
                    Label { text: qsTr("Type") }
                    ComboBox {
                        id: shapeTypeComboBox
                        function indexOf(value) {
                            var result = model.filter(function(element){ return element.value === value })
                            return result.length > 0 ? result[0].value : -1
                        }
                        model: [
                            { "text": qsTr("Sphere"),  "value": VPMM.RigidBody.SphereShape },
                            { "text": qsTr("Box"),     "value": VPMM.RigidBody.BoxShape },
                            { "text": qsTr("Capsule"), "value": VPMM.RigidBody.CapsureShape }
                        ]
                        currentIndex: indexOf(targetObject.shapeType)
                    }
                    Binding {
                        target: targetObject
                        property: "shapeType"
                        value: shapeTypeComboBox.model[shapeTypeComboBox.currentIndex].value
                        when: shapeTypeComboBox.hovered
                    }
                    Label { text: qsTr("Width") }
                    SpinBox {
                        id: rigidBodyWidthSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.mass
                    }
                    Binding {
                        target: targetObject
                        property: "width"
                        value: rigidBodyWidthSpinBox.value
                        when: rigidBodyWidthSpinBox.hovered
                    }
                    Label { text: qsTr("Height") }
                    SpinBox {
                        id: rigidBodyHeightSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.mass
                    }
                    Binding {
                        target: targetObject
                        property: "height"
                        value: rigidBodyHeightSpinBox.value
                        when: rigidBodyHeightSpinBox.hovered
                    }
                    Label { text: qsTr("Depth") }
                    SpinBox {
                        id: rigidBodyDepthSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.mass
                    }
                    Binding {
                        target: targetObject
                        property: "depth"
                        value: rigidBodyDepthSpinBox.value
                        when: rigidBodyDepthSpinBox.hovered
                    }
                }
            }
            GroupBox {
                title: qsTr("Collision Group")
                Layout.fillWidth: true
                ColumnLayout {
                    RowLayout {
                        Label { text: qsTr("Group Number") }
                        SpinBox { minimumValue: 0; maximumValue: 16 }
                    }
                    GridLayout {
                        columns: 4
                        CheckBox { text: qsTr("1") }
                        CheckBox { text: qsTr("2") }
                        CheckBox { text: qsTr("3") }
                        CheckBox { text: qsTr("4") }
                        CheckBox { text: qsTr("5") }
                        CheckBox { text: qsTr("6") }
                        CheckBox { text: qsTr("7") }
                        CheckBox { text: qsTr("8") }
                        CheckBox { text: qsTr("9") }
                        CheckBox { text: qsTr("10") }
                        CheckBox { text: qsTr("11") }
                        CheckBox { text: qsTr("12") }
                        CheckBox { text: qsTr("13") }
                        CheckBox { text: qsTr("14") }
                        CheckBox { text: qsTr("15") }
                        CheckBox { text: qsTr("16") }
                    }
                }
            }
            RowLayout {
                GroupBox {
                    title: qsTr("Position")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: rigidBodyPositionXSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: rigidBodyPosition.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: rigidBodyPositionYSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: rigidBodyPosition.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: rigidBodyPositionZSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: rigidBodyPosition.z
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Rotation")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: rigidBodyRotationXSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: rigidBodyRotation.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: rigidBodyRotationYSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: rigidBodyRotation.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: rigidBodyRotationZSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: rigidBodyRotation.z
                        }
                    }
                }
            }
            GroupBox {
                title: qsTr("Parameters")
                Layout.fillWidth: true
                GridLayout {
                    columns: 2
                    Label { text: qsTr("Mass") }
                    SpinBox {
                        id: rigidBodyMassSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.mass
                    }
                    Binding {
                        target: targetObject
                        property: "mass"
                        value: rigidBodyMassSpinBox.value
                        when: rigidBodyMassSpinBox.hovered
                    }
                    Label { text: qsTr("Linear Damping") }
                    SpinBox {
                        id: rigidBodyLinearDampingSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.linearDamping
                    }
                    Binding {
                        target: targetObject
                        property: "linearDamping"
                        value: rigidBodyLinearDampingSpinBox.value
                        when: rigidBodyLinearDampingSpinBox.hovered
                    }
                    Label { text: qsTr("Angular Damping") }
                    SpinBox {
                        id: rigidBodyAngularDampingSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.angularDamping
                    }
                    Binding {
                        target: targetObject
                        property: "angularDamping"
                        value: rigidBodyAngularDampingSpinBox.value
                        when: rigidBodyAngularDampingSpinBox.hovered
                    }
                    Label { text: qsTr("Restitution") }
                    SpinBox {
                        id: rigidBodyRestitutionSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.restitution
                    }
                    Binding {
                        target: targetObject
                        property: "restitution"
                        value: rigidBodyRestitutionSpinBox.value
                        when: rigidBodyRestitutionSpinBox.hovered
                    }
                    Label { text: qsTr("Friction") }
                    SpinBox {
                        id: rigidBodyFrictionSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: targetObject.friction
                    }
                    Binding {
                        target: targetObject
                        property: "friction"
                        value: rigidBodyFrictionSpinBox.value
                        when: rigidBodyFrictionSpinBox.hovered
                    }
                }
            }
            Item { height: 20 }
        }
    }
}
