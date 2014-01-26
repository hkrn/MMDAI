import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

ScrollView {
    id: jointView
    property var targetObject
    Item {
        id: jointContentView
        VPMM.Vector3 { id: jointPosition; value: targetObject.position }
        VPMM.Vector3 { id: jointRotation; value: targetObject.rotation }
        VPMM.Vector3 { id: jointPositionUpperLimit; value: targetObject.positionUpperLimit }
        VPMM.Vector3 { id: jointPositionLowerLimit; value: targetObject.positionLowerLimit }
        VPMM.Vector3 { id: jointRotationUpperLimit; value: targetObject.rotationUpperLimit }
        VPMM.Vector3 { id: jointRotationLowerLimit; value: targetObject.rotationLowerLimit }
        VPMM.Vector3 { id: jointPositionStiffness; value: targetObject.positionStiffness }
        VPMM.Vector3 { id: jointRotationStiffness; value: targetObject.rotationStiffness }
        Binding {
            target: targetObject
            property: "position"
            value: jointPosition.value
            when: jointPositionXSpinBox.hovered || jointPositionYSpinBox.hovered || jointPositionZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "rotation"
            value: jointRotation.value
            when: jointRotationXSpinBox.hovered || jointRotationYSpinBox.hovered || jointRotationZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "positionUpperLimit"
            value: jointPositionUpperLimit.value
            when: jointPositionUpperLimitXSpinBox.hovered || jointPositionUpperLimitYSpinBox.hovered || jointPositionUpperLimitZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "positionLowerLimit"
            value: jointPositionLowerLimit.value
            when: jointPositionLowerLimitXSpinBox.hovered || jointPositionLowerLimitYSpinBox.hovered || jointPositionLowerLimitZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "rotationUpperLimit"
            value: jointRotationUpperLimit.value
            when: jointRotationUpperLimitXSpinBox.hovered || jointRotationUpperLimitYSpinBox.hovered || jointRotationUpperLimitZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "rotationLowerLimit"
            value: jointRotationLowerLimit.value
            when: jointRotationLowerLimitXSpinBox.hovered || jointRotationLowerLimitYSpinBox.hovered || jointRotationLowerLimitZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "positionStiffness"
            value: jointPositionStiffness.value
            when: jointPositionStiffnessXSpinBox.hovered || jointPositionStiffnessYSpinBox.hovered || jointPositionStiffnessZSpinBox.hovered
        }
        Binding {
            target: targetObject
            property: "rotationStiffness"
            value: jointRotationStiffness.value
            when: jointRotationStiffnessXSpinBox.hovered || jointRotationStiffnessYSpinBox.hovered || jointRotationStiffnessZSpinBox.hovered
        }
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            Component.onCompleted: jointContentView.height = childrenRect.height
            GridLayout {
                columns: 2
                Label { text: qsTr("Name") }
                TextField {
                    id: jointNameTextField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Input Rigid Body Name Here")
                    text: targetObject.name
                }
                Binding {
                    target: targetObject
                    property: "name"
                    value: jointNameTextField.value
                    when: jointNameTextField.hovered
                }
                Label { text: qsTr("Rigid Body A") }
                ComboBox {
                    Layout.fillWidth: true
                    model: rigidBodiesModel
                    editable: true
                    currentIndex: targetObject.parentRigidBodyA ? targetObject.parentRigidBodyA.index : -1
                }
                Label { text: qsTr("Rigid Body B") }
                ComboBox {
                    Layout.fillWidth: true
                    model: rigidBodiesModel
                    editable: true
                    currentIndex: targetObject.parentRigidBodyB ? targetObject.parentRigidBodyB.index : -1
                }
                Label { text: qsTr("Type") }
                ComboBox {
                    id: jointTypeComboBox
                    function indexOf(value) {
                        var result = model.filter(function(element){ return element.value === value })
                        return result.length > 0 ? result[0].value : -1
                    }
                    model: [
                        { "text": qsTr("Spring 6-DOF"), "value": VPMM.Joint.Generic6DofSpringConstraint },
                        { "text": qsTr("Generic 6-DOF"), "value": VPMM.Joint.Generic6DofConstraint },
                        { "text": qsTr("Point to Point"), "value": VPMM.Joint.Point2PointConstraint },
                        { "text": qsTr("Cone Twist"), "value": VPMM.Joint.ConeTwistConstraint },
                        { "text": qsTr("Slider"), "value": VPMM.Joint.SliderConstraint },
                        { "text": qsTr("Hinge"), "value": VPMM.Joint.HingeConstraint }
                    ]
                    currentIndex: indexOf(targetObject.type)
                }
                Binding {
                    target: targetObject
                    property: "type"
                    value: jointTypeComboBox.model[jointTypeComboBox.currentIndex].value
                    when: jointTypeComboBox.hovered
                }
            }
            RowLayout {
                GroupBox {
                    title: qsTr("Position")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: jointPositionXSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: jointPosition.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: jointPositionYSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: jointPosition.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: jointPositionZSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: jointPosition.z
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Rotation")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: jointRotationXSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: jointRotation.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: jointRotationYSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: jointRotation.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: jointRotationZSpinBox
                            maximumValue: 180
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: jointRotation.z
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                GroupBox {
                    title: qsTr("Upper Limit")
                    RowLayout {
                        GroupBox {
                            title: qsTr("Translation")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: jointPositionUpperLimitXSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionUpperLimit.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: jointPositionUpperLimitYSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionUpperLimit.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: jointPositionUpperLimitZSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionUpperLimit.z
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("Orientation")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: jointRotationUpperLimitXSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationUpperLimit.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: jointRotationUpperLimitYSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationUpperLimit.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: jointRotationUpperLimitZSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationUpperLimit.z
                                }
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                GroupBox {
                    title: qsTr("Lower Limit")
                    RowLayout {
                        GroupBox {
                            title: qsTr("Translation")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: jointPositionLowerLimitXSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionLowerLimit.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: jointPositionLowerLimitYSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionLowerLimit.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: jointPositionLowerLimitZSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionLowerLimit.z
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("Orientation")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: jointRotationLowerLimitXSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationLowerLimit.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: jointRotationLowerLimitYSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationLowerLimit.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: jointRotationLowerLimitZSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationLowerLimit.z
                                }
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                GroupBox {
                    title: qsTr("Spring")
                    RowLayout {
                        GroupBox {
                            title: qsTr("Translation")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: jointPositionStiffnessXSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionStiffness.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: jointPositionStiffnessYSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionStiffness.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: jointPositionStiffnessZSpinBox
                                    maximumValue: 100000
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointPositionStiffness.z
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("Orientation")
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: jointRotationStiffnessXSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationStiffness.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: jointRotationStiffnessYSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationStiffness.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: jointRotationStiffnessZSpinBox
                                    maximumValue: 180
                                    minimumValue: -maximumValue
                                    decimals: 5
                                    stepSize: 0.01
                                    value: jointRotationStiffness.z
                                }
                            }
                        }
                    }
                }
            }
            Item { height: 20 }
        }
    }
}
