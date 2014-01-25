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
                    text: boneView.targetObject.name
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
                    exclusiveGroup: destinationTypeGroup
                }
                RadioButton {
                    id: destinationTypePosition
                    text: qsTr("Position")
                    checked: true
                    exclusiveGroup: destinationTypeGroup
                }
            }
            RowLayout {
                enabled: destinationTypeGroup.current === destinationTypeBone
                Label { text: qsTr("Destination Bone") }
                ComboBox {
                    model: bonesModel
                    editable: true
                    Layout.fillWidth: true
                    currentIndex: targetObject.destinationBone.index
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
                    enabled: destinationTypeGroup.current === destinationTypePosition
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
                        checked: boneView.targetObject.rotateable
                    }
                    Binding {
                        target: targetObject
                        property: "rotateable"
                        value: boneRotateableCheckbox.checked
                    }
                    CheckBox {
                        id: boneMovableCheckbox
                        text: qsTr("Movable")
                        checked: boneView.targetObject.movable
                    }
                    Binding {
                        target: targetObject
                        property: "movable"
                        value: boneMovableCheckbox.checked
                    }
                    CheckBox {
                        id: boneVisibleCheckbox
                        text: qsTr("Visible")
                        checked: boneView.targetObject.visible
                    }
                    Binding {
                        target: targetObject
                        property: "visible"
                        value: boneVisibleCheckbox.checked
                    }
                    CheckBox {
                        id: boneInteractiveCheckbox
                        text: qsTr("Interactive")
                        checked: boneView.targetObject.interactive
                    }
                    Binding {
                        target: targetObject
                        property: "interactive"
                        value: boneInteractiveCheckbox.checked
                    }
                    CheckBox {
                        id: boneInherenceOrientationCheckbox
                        text: qsTr("Inherence Orientation")
                        checked: boneView.targetObject.inherenceOrientationEnabled
                    }
                    Binding {
                        target: targetObject
                        property: "inherenceOrientation"
                        value: boneInherenceOrientationCheckbox.checked
                    }
                    CheckBox {
                        id: boneInherenceTranslationCheckbox
                        text: qsTr("Inherence Translation")
                        checked: boneView.targetObject.inherenceTranslationEnabled
                    }
                    Binding {
                        target: targetObject
                        property: "inherenceTranslation"
                        value: boneInherenceTranslationCheckbox.checked
                    }
                }
            }
            GroupBox {
                title: qsTr("Inherence")
                enabled: boneInherenceTranslationCheckbox.checked || boneInherenceOrientationCheckbox.checked
                Layout.fillWidth: true
                GridLayout {
                    columns: 2
                    Label { text: qsTr("Bone") }
                    ComboBox {
                        model: bonesModel
                        editable: true
                        Layout.fillWidth: true
                    }
                    Label { text: qsTr("Coefficient") }
                    SpinBox {
                        id: boneCoefficientSpinBox
                        maximumValue: 100
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: boneView.targetObject.coefficient
                    }
                    Binding {
                        target: targetObject
                        property: "coefficient"
                        value: boneCoefficientSpinBox.value
                        when: boneCoefficientSpinBox.hovered
                    }
                }
            }
            GroupBox {
                id: boneLocalAxesGroupBox
                title: qsTr("Local Axes")
                enabled: checked
                checkable: true
                checked: boneView.targetObject.localAxesEnabled
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
                enabled: checked
                checkable: true
                checked: boneView.targetObject.fixedAxisEnabled
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
                enabled: checked
                checkable: true
                checked: boneView.targetObject.inverseKinematicsEnabled
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
