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
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

ScrollView {
    id: materialView
    readonly property int colorPreviewWidth: 75
    readonly property int colorPreviewHeight: 25
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
                    text: targetObject.name
                }
            }
            RowLayout {
                Label { text: qsTr("Polygon Type") }
                ComboBox { model: [ "Triangle", "Line", "Point" ] }
                CheckBox {
                    id: materialVisibleCheckBox
                    text: "Visible"
                    checked: targetObject.visible
                }
                Binding {
                    target: targetObject
                    property: "visible"
                    value: materialVisibleCheckBox.checked
                }
            }
            GroupBox {
                title: qsTr("Color")
                Layout.fillWidth: true
                GridLayout {
                    id: materialColorLayout
                    columns: 8
                    RowLayout {
                        Layout.columnSpan: materialColorLayout.columns
                        Rectangle {
                            id: ambientColor
                            width: colorPreviewWidth
                            height: colorPreviewHeight
                            color: targetObject.ambient
                            ColorDialog {
                                id: ambientColorDialog
                                property color previousColor
                                title: qsTr("Ambient Color")
                                onCurrentColorChanged: targetObject.ambient = currentColor
                                onAccepted: {
                                    targetObject.ambient = color
                                    close()
                                }
                                onRejected: {
                                    targetObject.ambient = previousColor
                                    close()
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    ambientColorDialog.color = ambientColorDialog.previousColor = targetObject.ambient
                                    ambientColorDialog.open()
                                }
                            }
                        }
                        Label { text: qsTr("Ambient") }
                    }
                    Label { text: "R" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: ambientColor.color.r
                    }
                    Label { text: "G" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: ambientColor.color.g
                    }
                    Label { text: "B" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: ambientColor.color.b
                    }
                    Item {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        Layout.columnSpan: materialColorLayout.columns
                        Rectangle {
                            id: diffuseColor
                            width: colorPreviewWidth
                            height: colorPreviewHeight
                            color: targetObject.diffuse
                            ColorDialog {
                                id: diffuseColorDialog
                                property color previousColor
                                title: qsTr("Diffuse Color")
                                showAlphaChannel: true
                                onCurrentColorChanged: targetObject.diffuse = currentColor
                                onAccepted: {
                                    targetObject.diffuse = color
                                    close()
                                }
                                onRejected: {
                                    targetObject.diffuse = previousColor
                                    close()
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    diffuseColorDialog.color = diffuseColorDialog.previousColor = targetObject.diffuse
                                    diffuseColorDialog.open()
                                }
                            }
                        }
                        Label { text: qsTr("Diffuse") }
                    }
                    Label { text: "R" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: diffuseColor.color.r
                    }
                    Label { text: "G" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: diffuseColor.color.g
                    }
                    Label { text: "B" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: diffuseColor.color.b
                    }
                    Label { text: "A" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: diffuseColor.color.a
                    }
                    RowLayout {
                        Layout.columnSpan: materialColorLayout.columns
                        Rectangle {
                            id: specularColor
                            width: colorPreviewWidth
                            height: colorPreviewHeight
                            color: targetObject.specular
                            ColorDialog {
                                id: specularColorDialog
                                property color previousColor
                                title: qsTr("Specular Color")
                                onCurrentColorChanged: targetObject.specular = currentColor
                                onAccepted: {
                                    targetObject.specular = color
                                    close()
                                }
                                onRejected: {
                                    targetObject.specular = previousColor
                                    close()
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    specularColorDialog.color = specularColorDialog.previousColor = targetObject.specular
                                    specularColorDialog.open()
                                }
                            }
                        }
                        Label { text: qsTr("Specular") }
                    }
                    Label { text: "R" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: specularColor.color.r
                    }
                    Label { text: "G" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: specularColor.color.g
                    }
                    Label { text: "B" }
                    SpinBox {
                        minimumValue: 0
                        maximumValue: 1
                        decimals: 3
                        stepSize: 0.01
                        value: specularColor.color.b
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                    }
                    RowLayout {
                        Layout.columnSpan: materialColorLayout.columns
                        Label {
                            text: qsTr("Shininess")
                        }
                        SpinBox {
                            minimumValue: 0
                            maximumValue: 10000
                            decimals: 3
                            stepSize: 0.01
                            value: targetObject.shininess
                        }
                    }
                }
            }
            GroupBox {
                title: qsTr("Edge")
                checkable: true
                checked: targetObject.edgeEnabled
                Layout.fillWidth: true
                ColumnLayout {
                    GridLayout {
                        id: materialEdgeLayout
                        columns: 8
                        RowLayout {
                            Layout.columnSpan: materialEdgeLayout.columns
                            Rectangle {
                                id: edgeColor
                                width: colorPreviewWidth
                                height: colorPreviewHeight
                                color: targetObject.edgeColor
                                ColorDialog {
                                    id: edgeColorDialog
                                    property color previousColor
                                    title: qsTr("Edge Color")
                                    showAlphaChannel: true
                                    onCurrentColorChanged: targetObject.edgeColor = currentColor
                                    onAccepted: {
                                        targetObject.edgeColor = color
                                        close()
                                    }
                                    onRejected: {
                                        targetObject.edgeColor = previousColor
                                        close()
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        edgeColorDialog.color = edgeColorDialog.previousColor = targetObject.edgeColor
                                        edgeColorDialog.open()
                                    }
                                }
                            }
                            Label { text: qsTr("Edge") }
                        }
                        Label { text: "R" }
                        SpinBox {
                            minimumValue: 0
                            maximumValue: 1
                            decimals: 3
                            stepSize: 0.01
                            value: edgeColor.color.r
                        }
                        Label { text: "G" }
                        SpinBox {
                            minimumValue: 0
                            maximumValue: 1
                            decimals: 3
                            stepSize: 0.01
                            value: edgeColor.color.g
                        }
                        Label { text: "B" }
                        SpinBox {
                            minimumValue: 0
                            maximumValue: 1
                            decimals: 3
                            stepSize: 0.01
                            value: edgeColor.color.b
                        }
                        Label { text: "A" }
                        SpinBox {
                            minimumValue: 0
                            maximumValue: 1
                            decimals: 3
                            stepSize: 0.01
                            value: edgeColor.color.a
                        }
                    }
                    RowLayout {
                        Label { text: qsTr("Size") }
                        SpinBox {
                            height: 2
                            decimals: 3
                            stepSize: 0.01
                            value: targetObject.edgeSize
                        }
                    }
                }
            }
            GroupBox {
                title: qsTr("Capabilities")
                Layout.fillWidth: true
                ColumnLayout {
                    CheckBox {
                        text: qsTr("TwoSide Drawing")
                        checked: targetObject.cullingDisabled
                    }
                    CheckBox {
                        text: qsTr("Casting Projective Shadow")
                        checked: targetObject.castingShadowEnabled
                    }
                    CheckBox {
                        text: qsTr("Casting Shadow Map")
                        checked: targetObject.castingShadowMapEnabled
                    }
                    CheckBox {
                        text: qsTr("Enable Shadow Map")
                        checked: targetObject.shadowMapEnabled
                    }
                    CheckBox {
                        text: qsTr("Vertex Color")
                        checked: targetObject.vertexColorEnabled
                    }
                }
            }
            GroupBox {
                title: qsTr("Textures")
                Layout.fillWidth: true
                ColumnLayout {
                    GridLayout {
                        columns: 3
                        Label { text: qsTr("Main") }
                        TextField {
                            Layout.fillWidth: true
                            text: targetObject.mainTexturePath
                        }
                        Button { text: qsTr("Change")  }
                        Label { text: qsTr("Toon") }
                        TextField {
                            Layout.fillWidth: true
                            enabled: !enableSharedToonCheckbox.checked
                            text: targetObject.toonTexturePath
                        }
                        Button { text: qsTr("Change") }
                        Label { text: qsTr("Sphere") }
                        TextField {
                            Layout.fillWidth: true
                            text: targetObject.sphereTexturePath
                        }
                        Button { text: qsTr("Change") }
                    }
                    GridLayout {
                        columns: 2
                        CheckBox {
                            id: enableSharedToonCheckbox
                            text: qsTr("Enable Shared Toon")
                            checked: targetObject.sharedToonTextureEnabled
                        }
                        ComboBox {
                            id: toonTextureIndexComboBox
                            enabled: enableSharedToonCheckbox.checked
                            model: [
                                "toon1.bmp",
                                "toon2.bmp",
                                "toon3.bmp",
                                "toon4.bmp",
                                "toon5.bmp",
                                "toon6.bmp",
                                "toon7.bmp",
                                "toon8.bmp",
                                "toon9.bmp",
                                "toon10.bmp"
                            ]
                            currentIndex: enabled ? targetObject.toonTextureIndex : 0
                        }
                        Binding {
                            target: targetObject
                            property: "toonTextureIndex"
                            value: toonTextureIndexComboBox.enabled ? toonTextureIndexComboBox.currentIndex : 0
                        }
                        Label { text: qsTr("Sphere Texture Type") }
                        ComboBox {
                            id: sphereTextureTypeComboBox
                            function indexOf(value) {
                                var result = model.filter(function(element){ return element.value === value })
                                return result.length > 0 ? result[0].value : -1
                            }
                            model: [
                                { "text": qsTr("None"), "value": VPMM.Material.None },
                                { "text": qsTr("Multiply"), "value": VPMM.Material.Multiply },
                                { "text": qsTr("Additive"), "value": VPMM.Material.Additive },
                                { "text": qsTr("SubTexture"), "value": VPMM.Material.SubTexture }
                            ]
                            currentIndex: indexOf(targetObject.sphereTextureType)
                        }
                        Binding {
                            target: targetObject
                            property: "sphereTextureType"
                            value: sphereTextureTypeComboBox.model[sphereTextureTypeComboBox.currentIndex].value
                        }
                    }
                }
            }
            GroupBox {
                title: qsTr("Memo")
                Layout.fillWidth: true
                TextArea {
                    anchors.fill: parent
                    text: targetObject.userAreaData
                }
            }
            Item { height: 20 }
        }
    }
}
