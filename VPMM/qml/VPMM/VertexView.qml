import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

Item {
    id: vertexView
    property var targetObject
    VPMM.Vector3 { id: vertexOrigin; value: targetObject.origin }
    VPMM.Vector3 { id: vertexNormal; value: targetObject.normal }
    VPMM.Vector3 { id: vertexTextureCoord; value: targetObject.textureCoord }
    VPMM.Vector3 { id: vertexSdefC; value: targetObject.sdefC }
    VPMM.Vector3 { id: vertexSdefR0; value: targetObject.sdefR0 }
    VPMM.Vector3 { id: vertexSdefR1; value: targetObject.sdefR1 }
    Binding {
        target: targetObject
        property: "origin"
        value: vertexOrigin.value
        when: vertexOriginXSpinBox.hovered || vertexOriginYSpinBox.hovered || vertexOriginZSpinBox.hovered
    }
    Binding {
        target: targetObject
        property: "normal"
        value: vertexNormal.value
        when: vertexNormalXSpinBox.hovered || vertexNormalYSpinBox.hovered || vertexNormalZSpinBox.hovered
    }
    Binding {
        target: targetObject
        property: "textureCoord"
        value: vertexTextureCoord.value
        when: vertexTextureCoordUSpinBox.hovered || vertexTextureCoordVSpinBox.hovered
    }
    Binding {
        target: targetObject
        property: "sdefC"
        value: vertexSdefC.value
    }
    Binding {
        target: targetObject
        property: "sdefR0"
        value: vertexSdefR0.value
    }
    Binding {
        target: targetObject
        property: "sdefR1"
        value: vertexSdefR1.value
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        RowLayout {
            GroupBox {
                title: qsTr("Origin")
                GridLayout {
                    columns: 2
                    Label { text: "X" }
                    SpinBox {
                        id: vertexOriginXSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: vertexOrigin.x
                    }
                    Label { text: "Y" }
                    SpinBox {
                        id: vertexOriginYSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: vertexOrigin.y
                    }
                    Label { text: "Z" }
                    SpinBox {
                        id: vertexOriginZSpinBox
                        maximumValue: 100000
                        minimumValue: -maximumValue
                        decimals: 5
                        stepSize: 0.01
                        value: vertexOrigin.z
                    }
                }
            }
            GroupBox {
                title: qsTr("Normal")
                GridLayout {
                    columns: 2
                    Label { text: "X" }
                    SpinBox {
                        id: vertexNormalXSpinBox
                        maximumValue: 1
                        minimumValue: 0
                        decimals: 5
                        stepSize: 0.01
                        value: vertexNormal.x
                    }
                    Label { text: "Y" }
                    SpinBox {
                        id: vertexNormalYSpinBox
                        maximumValue: 1
                        minimumValue: 0
                        decimals: 5
                        stepSize: 0.01
                        value: vertexNormal.y
                    }
                    Label { text: "Z" }
                    SpinBox {
                        id: vertexNormalZSpinBox
                        maximumValue: 1
                        minimumValue: 0
                        decimals: 5
                        stepSize: 0.01
                        value: vertexNormal.z
                    }
                }
            }
        }
        GroupBox {
            title: qsTr("TexCoord")
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: qsTr("Type") }
                    ComboBox {
                        id: uvType
                        model: uvTypeModel
                    }
                }
                ColumnLayout {
                    RowLayout {
                        visible: uvTypeModel.get(uvType.currentIndex).value === 0
                        Label { text: "U" }
                        SpinBox {
                            id: vertexTextureCoordUSpinBox
                            maximumValue: 1
                            minimumValue: 0
                            decimals: 5
                            stepSize: 0.01
                            value: targetObject.textureCoord.x
                        }
                        Label { text: "V" }
                        SpinBox {
                            id: vertexTextureCoordVSpinBox
                            maximumValue: 1
                            minimumValue: 0
                            decimals: 5
                            stepSize: 0.01
                            value: targetObject.textureCoord.y
                        }
                    }
                    GridLayout {
                        columns: 4
                        visible: uvTypeModel.get(uvType.currentIndex).value > 0
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
                        Label { text: "W" }
                        SpinBox {
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                        }
                    }
                }
            }
        }
        GroupBox {
            title: qsTr("Edge")
            Layout.fillWidth: true
            RowLayout {
                Label { text: qsTr("Coefficient") }
                SpinBox {
                    id: vertexEdgeSizeSpinBox
                    maximumValue: 100
                    minimumValue: 0
                    decimals: 5
                    stepSize: 0.01
                    value: targetObject.edgeSize
                }
                Binding {
                    target: targetObject
                    property: "edgeSize"
                    value: vertexEdgeSizeSpinBox.value
                    when: vertexEdgeSizeSpinBox.hovered
                }
            }
        }
        GroupBox {
            title: qsTr("Weight")
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: qsTr("Type") }
                    ComboBox {
                        id: transformType
                        model: transformTypeModel
                        currentIndex: {
                            console.log([ targetObject.type, transformTypeModel.indexOf(targetObject.type) ])
                            return transformTypeModel.indexOf(targetObject.type)
                        }
                    }
                }
                ColumnLayout {
                    RowLayout {
                        ComboBox {
                            model: bonesModel
                            editable: true
                            currentIndex: targetObject.bone(0).index
                            Layout.fillWidth: targetObject.type === VPMM.Vertex.Bdef1
                        }
                        SpinBox {
                            visible: transformTypeModel.get(transformType.currentIndex).value !== 0
                            maximumValue: 1
                            minimumValue: 0
                            decimals: 5
                            stepSize: 0.01
                            value: targetObject.weight(0)
                            onValueChanged: targetObject.setWeight(0, value)
                        }
                    }
                    RowLayout {
                        visible: transformTypeModel.get(transformType.currentIndex).value >= 1
                        ComboBox {
                            model: bonesModel
                            editable: true
                            currentIndex: targetObject.bone(1).index
                        }
                        SpinBox {
                            maximumValue: 1
                            minimumValue: 0
                            decimals: 5
                            stepSize: 0.01
                            value: targetObject.type === VPMM.Vertex.Bdef2 ? 1.0 - targetObject.weight(0) : targetObject.weight(1)
                            onValueChanged: targetObject.setWeight(1, value)
                        }
                    }
                    RowLayout {
                        visible: transformTypeModel.get(transformType.currentIndex).value >= 3
                        ComboBox {
                            model: bonesModel
                            editable: true
                            currentIndex: targetObject.bone(2).index
                        }
                        SpinBox {
                            maximumValue: 1
                            minimumValue: 0
                            decimals: 5
                            stepSize: 0.01
                            value: targetObject.weight(2)
                            onValueChanged: targetObject.setWeight(2, value)
                        }
                    }
                    RowLayout {
                        visible: transformTypeModel.get(transformType.currentIndex).value >= 3
                        ComboBox {
                            model: bonesModel
                            editable: true
                            currentIndex: targetObject.bone(3).index
                        }
                        SpinBox {
                            maximumValue: 1
                            minimumValue: 0
                            decimals: 5
                            stepSize: 0.01
                            value: targetObject.weight(3)
                            onValueChanged: targetObject.setWeight(3, value)
                        }
                    }
                    GridLayout {
                        visible: transformTypeModel.get(transformType.currentIndex).value === 2
                        columns: 2
                        Label { text: "C" }
                        SpinBox {
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                        }
                        Label { text: "R0" }
                        SpinBox {
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                        }
                        Label { text: "R1" }
                        SpinBox {
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                        }
                    }
                }
            }
        }
        Item { Layout.fillHeight: true }
    }
}
