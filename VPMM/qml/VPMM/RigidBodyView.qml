import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

ScrollView {
    id: rigidBodyView
    property var targetObject
    /*
    onTargetObjectChanged: {
        var i = 0, l = 0
        objectTypeComboBox.currentIndex = -1
        console.log([ i, objectTypeModel, objectTypeModel.count ])
        for (i = 0, l = objectTypeModel.count; i < l; i++) {
            console.log([ i, objectTypeModel.model[i].value ])
            if (objectTypeModel.get(i).value === targetObject.objectType) {
                objectTypeComboBox.currentIndex = i
                break
            }
        }
        shapeTypeComboBox.currentIndex = -1
        for (i = 0, l = shapeTypeModel.count; i < l; i++) {
            if (shapeTypeModel.get(i).value === targetObject.shapeType) {
                shapeTypeComboBox.currentIndex = i
                break
            }
        }
    }
    */
    Item {
        id: rigidBodyContentView
        VPMM.Vector3 { id: rigidBodyPosition; value: targetObject.position }
        VPMM.Vector3 { id: rigidBodyRotation; value: targetObject.rotation }
        ListModel {
            id: objectTypeModel
            Component.onCompleted: {
                append({ "text": qsTr("Dynamic"), "value": VPMM.RigidBody.DynamicObject })
                append({ "text": qsTr("Static"),  "value": VPMM.RigidBody.StaticObject })
                append({ "text": qsTr("Aligned"), "value": VPMM.RigidBody.AlignedObject })
            }
        }
        ListModel {
            id: shapeTypeModel
            Component.onCompleted: {
                append({ "text": qsTr("Sphere"),  "value": VPMM.RigidBody.SphereShape })
                append({ "text": qsTr("Box"),     "value": VPMM.RigidBody.BoxShape })
                append({ "text": qsTr("Capsule"), "value": VPMM.RigidBody.CapsureShape })
            }
        }
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
                    text: rigidBodyView.targetObject.name
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
                    currentIndex: targetObject.parentBone.index
                }
                Binding {
                    target: targetObject
                    property: "bone"
                    value: targetObject.parentModel.allBones[rigidBodyBoneComboBox.currentIndex]
                }
                Label { text: qsTr("Type") }
                ComboBox {
                    id: objectTypeComboBox
                    model: objectTypeModel
                    onCurrentIndexChanged: targetObject.objectType = objectTypeModel.get(currentIndex).value
                }
                Binding {
                    target: targetObject
                    property: "objectType"
                    value: objectTypeModel.get(objectTypeComboBox.currentIndex).value
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
                        model: shapeTypeModel
                        onCurrentIndexChanged: targetObject.shapeType = shapeTypeModel.get(currentIndex).value
                    }
                    Binding {
                        target: targetObject
                        property: "shapeType"
                        value: shapeTypeModel.get(shapeTypeComboBox.currentIndex).value
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
