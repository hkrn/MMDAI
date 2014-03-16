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

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
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
                function __handleBoneSelected() {
                    modelObjectTypeBone.checked = true
                }
                function __handleMorphSelected() {
                    modelObjectTypeMorph.checked = true
                }
                function __handleModelChanged(model) {
                    if (model) {
                        model.firstTargetBoneChanged.connect(__handleBoneSelected)
                        model.firstTargetMorphChanged.connect(__handleMorphSelected)
                    }
                }
                enabled: scene.currentModel
                Component.onCompleted: scene.uploadingModelDidSucceed.connect(__handleModelChanged)
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
                    enabled: scene.currentModel
                    columns: 2
                    Label { text: qsTr("Scale") }
                    SpinBox {
                        id: modelScaleFactorSpinBox
                        minimumValue: 0.0
                        maximumValue: 1000.0
                        stepSize: 0.01
                        decimals: 2
                        value: scene.currentModel ? scene.currentModel.scaleFactor : 0
                    }
                    Binding {
                        target: scene.currentModel
                        property: "scaleFactor"
                        value: modelScaleFactorSpinBox.value
                        when: modelScaleFactorSpinBox.hovered
                    }
                    Label { text: qsTr("Opacity") }
                    SpinBox {
                        id: modelOpacitySpinBox
                        minimumValue: 0.0
                        maximumValue: 1.0
                        stepSize: 0.01
                        decimals: 2
                        value: scene.currentModel ? scene.currentModel.opacity : 0
                    }
                    Binding {
                        target: scene.currentModel
                        property: "opacity"
                        value: modelOpacitySpinBox.value
                        when: modelOpacitySpinBox.hovered
                    }
                    Label { text: qsTr("Edge") }
                    SpinBox {
                        id: modelEdgeWidthSpinBox
                        minimumValue: 0.0
                        maximumValue: 2.0
                        stepSize: 0.01
                        decimals: 2
                        value: 1.0
                    }
                    Binding {
                        target: scene.currentModel
                        property: "edgeWidth"
                        value: modelEdgeWidthSpinBox.value
                        when: modelEdgeWidthSpinBox.hovered
                    }
                    CheckBox {
                        id: modelVisibleCheckbox
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Visible")
                        checked: scene.currentModel ? scene.currentModel.visible : true
                    }
                    Binding {
                        target: scene.currentModel
                        property: "visible"
                        value: modelVisibleCheckbox.checked
                    }
                }
            }
            GroupBox {
                title: qsTr("Parent to bind")
                Layout.fillHeight: true
                ListModel {
                    id: parentBindingModelListModel
                    signal currentIndexWillChange(var index)
                    function __clearAvailableParentBindingModels() {
                        clear()
                        append({ "name": qsTr("None"), "model": null })
                    }
                    function __handleAvailableParentBindingModelsChanged() {
                        __clearAvailableParentBindingModels()
                        var models = scene.project.availableParentBindingModels,
                                currentModel = scene.currentModel,
                                selectedIndex = 0
                        for (var i in models) {
                            var model = models[i]
                            if (currentModel && currentModel.parentBindingModel === model) {
                                selectedIndex = Number(i) + 1
                            }
                            append({ "name": model.name, "model": model })
                        }
                        if (selectedIndex > 0) {
                            currentIndexWillChange(selectedIndex)
                        }
                    }
                    Component.onCompleted: {
                        scene.project.availableParentBindingModelsChanged.connect(__handleAvailableParentBindingModelsChanged)
                        __clearAvailableParentBindingModels()
                    }
                }
                ListModel {
                    id: parentBindingBoneListModel
                    signal currentIndexWillChange(var index)
                    function __clearAvailableParentBindingBones() {
                        clear()
                        append({ "name": qsTr("None"), "bone": null })
                    }
                    function __handleAvailableParentBindingBonesChanged() {
                        var bones = scene.project.availableParentBindingBones,
                                currentModel = scene.currentModel,
                                selectedIndex = 0
                        for (var i in bones) {
                            var bone = bones[i]
                            if (currentModel && currentModel.parentBindingBone === bone) {
                                selectedIndex = Number(i) + 1
                            }
                            append({ "name": bone.name, "bone": bone })
                        }
                        if (selectedIndex > 0) {
                            currentIndexWillChange(selectedIndex)
                        }
                    }
                    Component.onCompleted: {
                        scene.project.availableParentBindingBonesChanged.connect(__handleAvailableParentBindingBonesChanged)
                        __clearAvailableParentBindingBones()
                    }
                }
                ColumnLayout {
                    Label { text: qsTr("Model") }
                    ComboBox {
                        id: parentBindingModelComboBox
                        function __resetCurrentIndex() {
                            currentIndex = 0
                        }
                        function __handleCurrentIndexWillChange(index) {
                            currentIndex = index
                        }
                        model: parentBindingModelListModel
                        editable: true
                        textRole: "name"
                        onCurrentIndexChanged: {
                            var currentModel = scene.currentModel
                            if (currentModel) {
                                var targetModel = currentIndex > 0 ? parentBindingModelListModel.get(currentIndex).model : null
                                if (!targetModel) {
                                    currentModel.parentBindingBone = null
                                }
                                currentModel.parentBindingModel = targetModel
                                scene.project.updateParentBindingModel()
                            }
                        }
                        Component.onCompleted: {
                            scene.onCurrentModelChanged.connect(__resetCurrentIndex)
                            parentBindingModelListModel.currentIndexWillChange.connect(__handleCurrentIndexWillChange)
                        }
                    }
                    Label { text: qsTr("Bone") }
                    ComboBox {
                        id: parentBindingBoneComboBox
                        function __resetCurrentIndex() {
                            currentIndex = 0
                        }
                        function __handleCurrentIndexWillChange(index) {
                            currentIndex = index
                        }
                        enabled: parentBindingModelComboBox.currentIndex > 0
                        model: parentBindingBoneListModel
                        editable: true
                        textRole: "name"
                        onCurrentIndexChanged: {
                            var currentModel = scene.currentModel
                            if (currentModel) {
                                currentModel.parentBindingBone = currentIndex > 0 ? parentBindingBoneListModel.get(currentIndex).bone : null
                            }
                        }
                        Component.onCompleted: {
                            scene.onCurrentModelChanged.connect(__resetCurrentIndex)
                            parentBindingBoneListModel.currentIndexWillChange.connect(__handleCurrentIndexWillChange)
                        }
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
            Binding {
                target: scene.currentModel
                property: "translation"
                value: modelTranslationAxesSpinBox.value
                when: modelTranslationAxesSpinBox.hovered
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
                value: scene.currentModel ? scene.currentModel.eulerOrientation : Qt.vector3d(0, 0, 0)
                resettable: true
            }
            Binding {
                target: scene.currentModel
                property: "eulerOrientation"
                value: modelOrientationAxesSpinBox.value
                when: modelOrientationAxesSpinBox.hovered
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
                GroupBox {
                    id: modelBoneTransformType
                    title: qsTr("Transform")
                    ColumnLayout {
                        RowLayout {
                            visible: modelObjectType.isBone
                            enabled: scene.hasBoneSelected
                            ExclusiveGroup { id: tranformGroup }
                            RadioButton {
                                id: globalTransformMode
                                exclusiveGroup: tranformGroup
                                text: setTransformModeGlobalAction.text
                            }
                            Binding {
                                target: setTransformModeGlobalAction
                                property: "checked"
                                value: globalTransformMode.checked
                            }
                            RadioButton {
                                id: localTransformMode
                                exclusiveGroup: tranformGroup
                                text: setTransformModeLocalAction.text
                                checked: true
                            }
                            Binding {
                                target: setTransformModeLocalAction
                                property: "checked"
                                value: localTransformMode.checked
                            }
                            RadioButton {
                                id: viewTransformMode
                                exclusiveGroup: tranformGroup
                                text: setTransformModeViewAction.text
                            }
                            Binding {
                                target: setTransformModeViewAction
                                property: "checked"
                                value: viewTransformMode.checked
                            }
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
                CheckBox {
                    id: enableInverseKinematics
                    enabled: scene.hasBoneSelected && scene.currentModel.firstTargetBone.inverseKinematicsEnabled
                    checked: enabled && scene.currentModel.firstTargetBone.inverseKinematicsEnabled
                    text: qsTr("Enable IK (a.k.a Inverse Kinematics)")
                    onCheckedChanged: scene.currentModel.firstTargetBone.inverseKinematicsEnabled = checked
                }
                Item { Layout.fillHeight: true }
            }
            InterpolationPanel {
                targetKeyframe : (scene.hasBoneSelected && scene.currentMotion) ? scene.currentMotion.resolveKeyframeAt(timeline.timeIndex, scene.currentModel.firstTargetBone) : null
                enabled: targetKeyframe
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
                resettable: true
                value: scene.hasBoneSelected ? scene.currentModel.firstTargetBone.localTranslation : Qt.vector3d(0, 0, 0)
                onActiveFocusChanged: activeFocus ? scene.currentModel.beginTransform(0) : scene.currentModel.discardTransform()
                onEditingFinished: scene.currentModel.commitTransform()
                onResetDidTrigger: resetBoneXYZAxesTranslationAction.trigger()
            }
            Binding {
                target: scene.hasBoneSelected ? scene.currentModel.firstTargetBone : null
                property: "localTranslation"
                value: boneTranslationAxesSpinBox.value
                when: boneTranslationAxesSpinBox.hovered
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
                resettable: true
                value: scene.hasBoneSelected ? scene.currentModel.firstTargetBone.localEulerOrientation : Qt.vector3d(0, 0, 0)
                onActiveFocusChanged: activeFocus ? scene.currentModel.beginTransform(0) : scene.currentModel.discardTransform()
                onEditingFinished: scene.currentModel.commitTransform()
                onResetDidTrigger: resetBoneOrientationAction.trigger()
            }
            Binding {
                target: scene.hasBoneSelected ? scene.currentModel.firstTargetBone : null
                property: "localEulerOrientation"
                value: boneOrientationAxesSpinBox.value
                when: boneOrientationAxesSpinBox.hovered
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
                                id: morphCategory
                                function __handleFirstTargetMorphChanged() {
                                    var morph = scene.currentModel.firstTargetMorph
                                    if (morph) {
                                        var category = morph.category
                                        for (var i in model) {
                                            if (model[i].value === category) {
                                                currentIndex = i
                                                break
                                            }
                                        }
                                        morphList.currentIndex = -1 /* to update morphList forcely */
                                        morphList.currentIndex = morphList.model.indexOf(morph)
                                    }
                                }
                                function __handleUploadingModel(model) {
                                    if (model) {
                                        model.firstTargetMorphChanged.connect(__handleFirstTargetMorphChanged)
                                    }
                                }
                                Component.onCompleted: scene.uploadingModelDidSucceed.connect(__handleUploadingModel)
                                model: morphCategoryModel
                                onCurrentIndexChanged: {
                                    var currentModel = scene.currentModel
                                    if (currentModel) {
                                        var category = model[currentIndex].value
                                        morphList.model = currentModel.findMorphsByCategory(category)
                                        if (category === VPVM.Morph.Unknown) {
                                            currentModel.firstTargetMorph = null
                                        }
                                        else if (morphList.model.length > 0) {
                                            var morph = morphList.model[0]
                                            morphList.currentIndex = 0
                                            currentModel.firstTargetMorph = morph
                                        }
                                    }
                                }
                            }
                            ComboBox {
                                id: morphList
                                visible: morphCategory.model[morphCategory.currentIndex].value !== VPVM.Morph.Unknown
                                editable: true
                                textRole: "name"
                                onCurrentIndexChanged: {
                                    var currentModel = scene.currentModel
                                    if (currentIndex != -1 && currentModel) {
                                        var morph = model[currentIndex]
                                        if (morph) {
                                            currentModel.firstTargetMorph = morph
                                            morphSlider.value = morph.weight
                                        }
                                    }
                                }
                            }
                        }
                        ColumnLayout {
                            RowLayout {
                                Slider {
                                    id: morphSlider
                                    enabled: scene.hasMorphSelected
                                    minimumValue: 0
                                    maximumValue: 1.0
                                    tickmarksEnabled: true
                                    function updateKeyframe() {
                                        var motion = scene.currentMotion
                                        if (!hovered && motion) {
                                            var morph = scene.currentModel.firstTargetMorph
                                            var timeIndex = timeline.timeIndex
                                            motion.updateKeyframe(morph, timeIndex)
                                        }
                                    }
                                    onPressedChanged: if (!pressed) updateKeyframe()
                                    Binding on value {
                                        value: morphSpinBox.value
                                        when: morphSpinBox.hovered
                                    }
                                }
                                Binding {
                                    target: scene.hasMorphSelected ? scene.currentModel.firstTargetMorph : null
                                    property: "weight"
                                    value: morphSlider.value
                                    when: morphSlider.pressed
                                }
                                SpinBox {
                                    id: morphSpinBox
                                    enabled: scene.hasMorphSelected
                                    minimumValue: morphSlider.minimumValue
                                    maximumValue: morphSlider.maximumValue
                                    decimals: 3
                                    stepSize: 0.01
                                    onEditingFinished: morphSlider.updateKeyframe()
                                    Binding on value {
                                        value: morphSlider.value
                                        when: morphSlider.pressed
                                    }
                                }
                                Binding {
                                    target: scene.hasMorphSelected ? scene.currentModel.firstTargetMorph : null
                                    property: "weight"
                                    value: morphSpinBox.value
                                    when: morphSpinBox.hovered
                                }
                                function __handleUndoDidPerform() {
                                    if (scene.hasMorphSelected) {
                                        morphSpinBox.value = morphSlider.value = scene.currentModel.firstTargetMorph.weight
                                    }
                                }
                                function __handleCurrentTimeIndexChanged() {
                                    if (scene.hasMorphSelected) {
                                        var keyframe = scene.currentMotion.resolveKeyframeAt(scene.project.currentTimeIndex, scene.currentModel.firstTargetMorph)
                                        morphSpinBox.value = morphSlider.value = keyframe ? keyframe.weight : 0
                                        morphKeyframeRegisteredCheckbox.checked = keyframe ? true : false
                                    }
                                }
                                Component.onCompleted: {
                                    scene.project.onUndoDidPerform.connect(__handleUndoDidPerform)
                                    scene.project.onRedoDidPerform.connect(__handleUndoDidPerform)
                                    scene.project.currentTimeIndexChanged.connect(__handleCurrentTimeIndexChanged)
                                }
                            }
                            CheckBox {
                                id: morphKeyframeRegisteredCheckbox
                                checked: false
                                enabled: false
                                text: qsTr("Is Keyframe Registered?")
                            }
                        }
                    }
                }
            }
        }
        Item { Layout.fillWidth: true }
    }
}
