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

import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import com.github.mmdai.VPVM 1.0 as VPVM

Tab {
    id: modelTab
    enabled: scene.currentModel
    title: qsTr("Model")
    anchors.margins: propertyPanel.anchors.margins
    RowLayout {
        id: modelObjectGroup
        GroupBox {
            id: modelObjectTypeGroup
            Layout.fillHeight: true
            title: qsTr("Type")
            ColumnLayout {
                id: modelObjectType
                readonly property bool isModel: objectTypeGroup.current === modelObjectTypeModel
                readonly property bool isBone: objectTypeGroup.current === modelObjectTypeBone
                readonly property bool isMorph: objectTypeGroup.current === modelObjectTypeMorph
                enabled: scene.currentModel
                ExclusiveGroup { id: objectTypeGroup }
                RadioButton {
                    id: modelObjectTypeModel
                    checked: true
                    exclusiveGroup: objectTypeGroup
                    text: qsTr("Model")
                }
                RadioButton {
                    id: modelObjectTypeBone
                    exclusiveGroup: objectTypeGroup
                    text: qsTr("Bone")
                }
                RadioButton {
                    id: modelObjectTypeMorph
                    exclusiveGroup: objectTypeGroup
                    text: qsTr("Morph")
                }
            }
        }
        RowLayout {
            id: modelModelGroup
            visible: modelObjectTypeGroup.visible && modelObjectType.isModel
            GroupBox {
                Layout.fillHeight: true
                title: qsTr("Properties")
                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    Label { text: qsTr("Scale") }
                    SpinBox {
                        minimumValue: 0.0
                        maximumValue: 1000.0
                        stepSize: 0.01
                        decimals: 2
                        value: scene.currentModel ? scene.currentModel.scaleFactor : 1
                        onValueChanged: if (scene.currentModel) scene.currentModel.scaleFactor = value
                    }
                    Label { text: qsTr("Opacity") }
                    SpinBox {
                        minimumValue: 0.0
                        maximumValue: 1.0
                        stepSize: 0.01
                        decimals: 2
                        value: scene.currentModel ? scene.currentModel.opacity : 1
                        onValueChanged: if (scene.currentModel) scene.currentModel.opacity = value
                    }
                    Label { text: qsTr("Edge") }
                    SpinBox {
                        minimumValue: 0.0
                        maximumValue: 2.0
                        stepSize: 0.01
                        decimals: 2
                        value: scene.currentModel ? scene.currentModel.edgeWidth : 1
                        onValueChanged: if (scene.currentModel) scene.currentModel.edgeWidth = value
                    }
                    CheckBox {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Visible")
                        checked: scene.currentModel ? scene.currentModel.visible : true
                        onCheckedChanged: if (scene.currentModel) scene.currentModel.visible = checked
                    }
                }
            }
            GroupBox {
                title: qsTr("Parent to bind")
                Layout.fillHeight: true
                ColumnLayout {
                    Label { text: qsTr("Model") }
                    ComboBox {
                        id: parentBindingModelComboBox
                        enabled: false
                        /*
                          disable setting parent model/bone because of crash

                        model: scene.project.availableParentBindingModels
                        textRole: "name"
                        onCurrentIndexChanged: {
                            var currentModel = scene.currentModel
                            if (currentModel) {
                                currentModel.parentBindingModel = currentIndex > 0 ? model[currentIndex] : null
                                scene.project.updateParentBindingModel()
                            }
                        }

                        */
                    }
                    Label { text: qsTr("Bone") }
                    ComboBox {
                        id: parentBindingBoneComboBox
                        enabled: false
                        /*

                        enabled: parentBindingModelComboBox.currentIndex > 0
                        model: scene.project.availableParentBindingBones
                        textRole: "name"
                        onCurrentIndexChanged: {
                            var currentModel = scene.currentModel
                            if (currentModel) {
                                currentModel.parentBindingBone = currentIndex > 0 ? model[currentIndex] : null
                            }
                        }

                        */
                    }
                }
            }
            AxesSpinBox {
                id: modelTranslationAxesSpinBox
                Layout.fillHeight: true
                title: qsTr("Translation")
                visible: modelModelGroup.visible
                minimumValue: propertyPanel.minimumPositionValue
                maximumValue: propertyPanel.maximumPositionValue
                stepSize: propertyPanel.positionStepSize
                decimals: propertyPanel.positionDecimalPrecision
                value: scene.currentModel ? scene.currentModel.translation : Qt.vector3d(0, 0, 0)
                resettable: true
            }
            AxesSpinBox {
                id: modelOrientationAxesSpinBox
                Layout.fillHeight: true
                title: qsTr("Orientation")
                visible: modelModelGroup.visible
                minimumValue: propertyPanel.minimumRotaitonValue
                maximumValue: propertyPanel.maximumRotaitonValue
                stepSize: propertyPanel.rotationStepSize
                decimals: propertyPanel.rotationDecimalPrecision
                value: scene.currentModel ? scene.currentModel.orientation : Qt.vector3d(0, 0, 0)
                resettable: true
            }
        }
        RowLayout {
            id: modelBoneGroup
            visible: modelObjectTypeGroup.visible
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillHeight: true
                visible: modelObjectType.isBone
                GroupBox {
                    id: modelBoneTransformMode
                    title: qsTr("Mode")
                    RowLayout {
                        visible: modelObjectType.isBone
                        enabled: scene.hasBoneSelected
                        ExclusiveGroup { id: modeGroup }
                        RadioButton {
                            id: boneSelectMode
                            enabled: setSelectModeAction.enabled
                            exclusiveGroup: modeGroup
                            text: setSelectModeAction.text
                            checked: setSelectModeAction.checked
                            onCheckedChanged: if (checked) setSelectModeAction.trigger()
                        }
                        RadioButton {
                            id: boneMoveMode
                            enabled: setMoveModeAction.enabled
                            exclusiveGroup: modeGroup
                            text: setMoveModeAction.text
                            checked: setMoveModeAction.checked
                            onCheckedChanged: if (checked) setMoveModeAction.trigger()
                        }
                        RadioButton {
                            id: boneRotateMode
                            enabled: setRotateModeAction.enabled
                            exclusiveGroup: modeGroup
                            text: setRotateModeAction.text
                            checked: setRotateModeAction.checked
                            onCheckedChanged: if (checked) setRotateModeAction.trigger()
                        }
                    }
                }
                CheckBox {
                    id:  enableSnapGizmo
                    enabled: boneTranslationAxesSpinBox.visible
                    checked: scene.enableSnapGizmo
                    text: qsTr("Enable Gizmo with Snap")
                    onCheckedChanged: scene.enableSnapGizmo = checked
                }
                GroupBox {
                    id: modelBoneTransformType
                    title: qsTr("Transform")
                    RowLayout {
                        visible: modelObjectType.isBone
                        enabled: scene.hasBoneSelected
                        ExclusiveGroup { id: tranformGroup }
                        RadioButton {
                            id: globalTransformMode
                            exclusiveGroup: tranformGroup
                            text: setTransformModeGlobalAction.text
                            checked: setTransformModeGlobalAction.checked
                            onCheckedChanged: if (checked) setTransformModeGlobalAction.trigger()
                        }
                        RadioButton {
                            id: localTransformMode
                            exclusiveGroup: tranformGroup
                            text: setTransformModeLocalAction.text
                            checked: setTransformModeLocalAction.checked
                            onCheckedChanged: if (checked) setTransformModeLocalAction.trigger()
                        }
                        RadioButton {
                            id: viewTransformMode
                            exclusiveGroup: tranformGroup
                            text: setTransformModeViewAction.text
                            checked: setTransformModeViewAction.checked
                            onCheckedChanged: if (checked) setTransformModeViewAction.trigger()
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
            InterpolationPanel {
                targetKeyframe : (enabled && scene.currentMotion) ? scene.currentMotion.resolveKeyframeAt(timeline.timeIndex, scene.currentModel.firstTargetBone) : null
                enabled: scene.hasBoneSelected && scene.currentMotion
                visible: modelObjectType.isBone && (boneMoveMode.checked || boneRotateMode.checked)
                type: 0
                typeModel: [
                    qsTr("X Axis"),
                    qsTr("Y Axis"),
                    qsTr("Z Axis"),
                    qsTr("Rotation")
                ]
            }
            AxesSpinBox {
                id: boneTranslationAxesSpinBox
                Layout.fillHeight: true
                title: qsTr("Translation")
                visible: modelObjectType.isBone && boneMoveMode.checked
                enabled: scene.hasBoneSelected
                minimumValue: propertyPanel.minimumPositionValue
                maximumValue: propertyPanel.maximumPositionValue
                stepSize: propertyPanel.positionStepSize
                decimals: propertyPanel.positionDecimalPrecision
                value: enabled ? scene.currentModel.firstTargetBone.localTranslation : Qt.vector3d(0, 0, 0)
                resettable: true
                onHoveredChanged: {
                    if (enabled) {
                        hovered ? scene.currentModel.beginTranslate(0) : scene.currentModel.endTranslate()
                    }
                }
            }
            AxesSpinBox {
                id: boneOrientationAxesSpinBox
                Layout.fillHeight: true
                title: qsTr("Orientation")
                visible: modelObjectType.isBone && boneRotateMode.checked
                enabled: scene.hasBoneSelected
                minimumValue: propertyPanel.minimumRotaitonValue
                maximumValue: propertyPanel.maximumRotaitonValue
                stepSize: propertyPanel.rotationStepSize
                decimals: propertyPanel.rotationDecimalPrecision
                value: enabled ? scene.currentModel.firstTargetBone.localOrientation : Qt.vector3d(0, 0, 0)
                resettable: true
                onHoveredChanged: {
                    if (enabled) {
                        hovered ? scene.currentModel.beginRotate(0) : scene.currentModel.endRotate()
                    }
                }
            }
            RowLayout {
                id: modelMorphGroup
                visible: modelObjectType.isMorph
                GroupBox {
                    Layout.fillHeight: true
                    title: qsTr("Morph")
                    ColumnLayout {
                        RowLayout {
                            ComboBox {
                                id: category
                                model: [
                                    { "text": qsTr("None"), "category": VPVM.Morph.Unknown },
                                    { "text": qsTr("Eye"), "category": VPVM.Morph.Eye },
                                    { "text": qsTr("Lip"), "category": VPVM.Morph.Lip },
                                    { "text": qsTr("Eyeblow"), "category": VPVM.Morph.Eyeblow },
                                    { "text": qsTr("Other"), "category": VPVM.Morph.Other }
                                ]
                                onCurrentIndexChanged: {
                                    var currentModel = scene.currentModel
                                    if (currentModel) {
                                        var category = model[currentIndex].category
                                        morphList.model = currentModel.findMorphsByCategory(category)
                                        if (category === VPVM.Morph.Unknown) {
                                            currentModel.firstTargetMorph = null
                                        }
                                    }
                                }
                            }
                            ComboBox {
                                id: morphList
                                textRole: "name"
                                onCurrentIndexChanged: {
                                    var currentModel = scene.currentModel
                                    if (currentModel) {
                                        var morph = model[currentIndex]
                                        if (morph) {
                                            currentModel.firstTargetMorph = morph
                                        }
                                    }
                                }
                            }
                        }
                        RowLayout {
                            Slider {
                                id: morphSlider
                                minimumValue: 0
                                maximumValue: 1.0
                                onValueChanged: morphSpinBox.value = value
                            }
                            SpinBox {
                                id: morphSpinBox
                                enabled: scene.hasMorphSelected
                                minimumValue: morphSlider.minimumValue
                                maximumValue: morphSlider.maximumValue
                                decimals: 3
                                stepSize: 0.01
                                onValueChanged: morphSlider.value = value
                            }
                        }
                    }
                }
            }
        }
        Rectangle { Layout.fillWidth: true }
    }
}
