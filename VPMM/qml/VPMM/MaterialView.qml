import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

ScrollView {
    id: materialView
    property var targetObject
    Item {
        id: materialContentView
        ColumnLayout {
            anchors.margins: 12
            anchors.fill: parent
            Component.onCompleted: materialContentView.height = childrenRect.height
            RowLayout {
                Label { text: qsTr("Name") }
                TextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Input Material Name Here")
                    text: materialView.targetObject.name
                }
            }
            RowLayout {
                Label { text: qsTr("Polygon Type") }
                ComboBox { model: [ "Triangle", "Line", "Point" ] }
            }
            GroupBox {
                title: qsTr("Color")
                Layout.fillWidth: true
                GridLayout {
                    columns: 6
                    Label { Layout.columnSpan: 2; Layout.alignment: Qt.AlignCenter; text: qsTr("Preview") }
                    Label { text: qsTr("Red") }
                    Label { text: qsTr("Green") }
                    Label { text: qsTr("Blue") }
                    Label { text: qsTr("Alpha") }
                    Rectangle { id: ambientColor; width: 25; height: 25; color: materialView.targetObject.ambient }
                    Label { text: qsTr("Ambient") }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: ambientColor.color.r }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: ambientColor.color.g }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: ambientColor.color.b }
                    Item { Layout.fillWidth: true }
                    Rectangle { id: diffuseColor; width: 25; height: 25; color: materialView.targetObject.diffuse }
                    Label { text: qsTr("Diffuse") }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: diffuseColor.color.r }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: diffuseColor.color.g }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: diffuseColor.color.b }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: diffuseColor.color.a }
                    Rectangle { id: specularColor; width: 25; height: 25; color: materialView.targetObject.specular }
                    Label { text: qsTr("Specular") }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: specularColor.color.r }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: specularColor.color.r }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: specularColor.color.r }
                    Item { Layout.fillWidth: true }
                    Rectangle { width: 25; height: 25; color: systemPalette.window }
                    Label { text: qsTr("Shininess") }
                    SpinBox { minimumValue: 0; maximumValue: 255; value: materialView.targetObject.shininess }
                    Item { Layout.columnSpan: 3; Layout.fillWidth: true }
                }
            }
            GroupBox {
                title: qsTr("Edge")
                checkable: true
                checked: materialView.targetObject.edgeEnabled
                Layout.fillWidth: true
                ColumnLayout {
                    RowLayout {
                        Rectangle { id: edgeColor; width: 25; height: 25; color: materialView.targetObject.edgeColor }
                        Label { text: qsTr("Edge") }
                        SpinBox { minimumValue: 0; maximumValue: 255; value: edgeColor.color.r }
                        SpinBox { minimumValue: 0; maximumValue: 255; value: edgeColor.color.g }
                        SpinBox { minimumValue: 0; maximumValue: 255; value: edgeColor.color.b }
                        SpinBox { minimumValue: 0; maximumValue: 255; value: edgeColor.color.a }
                    }
                    RowLayout {
                        Label { text: qsTr("Size") }
                        SpinBox { height: 2; decimals: 3; stepSize: 0.01; value: materialView.targetObject.edgeSize }
                    }
                }
            }
            GroupBox {
                title: qsTr("Capabilities")
                Layout.fillWidth: true
                ColumnLayout {
                    CheckBox { text: qsTr("TwoSide Drawing"); checked: materialView.targetObject.cullingDisabled }
                    CheckBox { text: qsTr("Casting Projective Shadow"); checked: materialView.targetObject.castingShadowEnabled }
                    CheckBox { text: qsTr("Casting Shadow Map"); checked: materialView.targetObject.castingShadowMapEnabled }
                    CheckBox { text: qsTr("Enable Shadow Map"); checked: materialView.targetObject.shadowMapEnabled }
                    CheckBox { text: qsTr("Vertex Color"); checked: materialView.targetObject.vertexColorEnabled }
                }
            }
            GroupBox {
                title: qsTr("Textures")
                Layout.fillWidth: true
                ColumnLayout {
                    GridLayout {
                        columns: 3
                        Label { text: qsTr("Main") }
                        TextField { Layout.fillWidth: true; text: materialView.targetObject.mainTexturePath }
                        Button { text: qsTr("Change")  }
                        Label { text: qsTr("Toon") }
                        TextField { Layout.fillWidth: true; text: materialView.targetObject.sphereTexturePath }
                        Button { text: qsTr("Change") }
                        Label { text: qsTr("Sphere") }
                        TextField { Layout.fillWidth: true; text: materialView.targetObject.toonTexturePath }
                        Button { text: qsTr("Change") }
                    }
                    GridLayout {
                        columns: 2
                        CheckBox {
                            id: enableSharedToonCheckbox
                            text: qsTr("Enable Shared Toon")
                            checked: false
                        }
                        ComboBox {
                            enabled: enableSharedToonCheckbox.checked
                            model: [
                                0, 1, 2, 3, 4, 5, 6, 7, 8, 9
                            ]
                        }
                        Label { text: qsTr("Sphere Texture Type") }
                        ComboBox {
                            model: ListModel {
                                ListElement { text: "None" }
                                ListElement { text: "Multiply" }
                                ListElement { text: "Add" }
                                ListElement { text: "SubTexture" }
                            }
                        }
                    }
                }
            }
            GroupBox {
                title: qsTr("Memo")
                Layout.fillWidth: true
                TextArea {
                    anchors.fill: parent
                    text: materialView.targetObject.userAreaData
                }
            }
            Item { height: 20 }
        }
    }
}
