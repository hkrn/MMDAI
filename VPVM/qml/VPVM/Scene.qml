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
import com.github.mmdai.VPVM 1.0 as VPVM
import "FontAwesome.js" as FontAwesome

Item {
    id: scene
    readonly property alias project: projectDocument
    readonly property alias camera : projectDocument.camera
    readonly property alias light  : projectDocument.light
    readonly property alias viewport : renderTarget.viewport
    readonly property alias graphicsDevice : renderTarget.graphicsDevice
    readonly property int __cornerMarginSize : 5
    property int baseFontPointSize : 16
    property int baseIconPointSize : 48
    readonly property bool hasBoneSelected : projectDocument.currentModel && projectDocument.currentModel.firstTargetBone
    readonly property bool hasMorphSelected : projectDocument.currentModel && projectDocument.currentModel.firstTargetMorph
    readonly property bool playing: state === "play" || state === "export"
    readonly property bool encoding: state === "encode"
    property alias currentModel : projectDocument.currentModel
    property alias currentMotion : projectDocument.currentMotion
    property alias enableSnapGizmo : renderTarget.enableSnapGizmo
    property alias snapGizmoStepSize : renderTarget.snapGizmoStepSize
    property alias grid : renderTarget.grid
    property alias editMode : renderTarget.editMode
    property bool isHUDAvailable : true
    property bool loop : false
    property real offsetY : 0
    signal toggleTimelineVisible()
    signal toggleTimelineWindowed()
    signal modelDidUpload(var model)
    signal errorDidHappen(string message)
    signal currentTimeIndexDidChange(int timeIndex)
    signal boneTransformTypeDidChange(int type)
    signal boneDidSelect(var bone)
    signal morphDidSelect(var morph)

    Component.onCompleted: {
        if (Qt.platform.os !== "osx") {
            baseFontPointSize = 12
            baseIconPointSize = 36
        }
    }

    Loader {
        id: confirmWindowLoader
        asynchronous: true
        visible: status === Loader.Ready
        function __handleAccept() {
            progressBar.indeterminate = true
            progressBar.visible = true
            renderTarget.forceActiveFocus()
        }
        function __handleReject() {
            progressBar.visible = false
            renderTarget.forceActiveFocus()
        }
        onLoaded: {
            item.accept.connect(__handleAccept)
            item.reject.connect(__handleReject)
            item.show()
        }
    }
    Loader {
        id: videoLoader
        function loadSource(source) {
            item.videoSource = source
        }
        function seek(position) {
            if (status === Loader.Ready) {
                item.seek(position)
            }
        }
        anchors.centerIn: renderTarget
        asynchronous: true
        visible: status === Loader.Ready
        onLoaded: loadSource(item.videoSource)
    }

    function undo() {
        projectDocument.undo()
        renderTarget.render()
    }
    function redo() {
        projectDocument.redo()
        renderTarget.render()
    }
    function seek(timeIndex) {
        projectDocument.seek(timeIndex)
        videoLoader.seek(projectDocument.secondsFromTimeIndex(timeIndex))
        renderTarget.render()
    }
    function seekNextTimeIndex(step) {
        projectDocument.seek(projectDocument.currentTimeIndex + step)
        renderTarget.render()
    }
    function seekPreviousTimeIndex(step) {
        projectDocument.seek(projectDocument.currentTimeIndex - step)
        renderTarget.render()
    }
    function resetBone(opaque, type) {
        projectDocument.resetBone(opaque, type)
        renderTarget.render()
    }
    function resetMorph(opaque) {
        projectDocument.resetMorph(opaque)
        renderTarget.render()
    }
    function loadVideo(fileUrl) {
        if (videoLoader.status === Loader.Ready) {
            videoLoader.loadSource(fileUrl)
        }
        else {
            videoLoader.setSource("Video.qml", { "videoSource": fileUrl })
        }
    }
    function exportImage(fileUrl, size) {
        renderTarget.exportImage(fileUrl, size)
    }
    function __handleEncodeDidFinish() {
        state = "stop"
    }
    function exportVideo(fileUrl) {
        state = "export"
        renderTarget.exportVideo(fileUrl)
    }
    function cancelExportingVideo() {
        renderTarget.cancelExportingVideo()
    }
    function setRange(from, to) {
        renderTargetAnimation.setRange(from, to)
    }

    states: [
        State {
            name: "play"
            PropertyChanges { target: scene; isHUDAvailable: false }
            StateChangeScript {
                script: {
                    renderTargetAnimation.start()
                    standbyRenderTimer.stop()
                }
            }
        },
        State {
            name: "export"
            extend: "play"
        },
        State {
            name: "pause"
            PropertyChanges { target: scene; isHUDAvailable: true }
            StateChangeScript {
                script: {
                    renderTargetAnimation.stop()
                    standbyRenderTimer.start()
                    renderTarget.lastTimeIndex = projectDocument.currentTimeIndex
                }
            }
        },
        State {
            name: "stop"
            PropertyChanges { target: scene; isHUDAvailable: true }
            StateChangeScript {
                script: {
                    renderTargetAnimation.stop()
                    renderTarget.currentTimeIndex = renderTarget.lastTimeIndex = 0
                    projectDocument.rewind()
                    renderTarget.render()
                    standbyRenderTimer.start()
                }
            }
        },
        State {
            name: "encode"
            PropertyChanges { target: scene; isHUDAvailable: false }
            StateChangeScript {
                script: {
                    renderTargetAnimation.stop()
                    renderTarget.currentTimeIndex = renderTarget.lastTimeIndex = 0
                    projectDocument.rewind()
                    renderTarget.render()
                }
            }
        }
    ]

    VPVM.Project {
        id: projectDocument
        property bool __constructing: false
        function __stopProject() {
            standbyRenderTimer.stop()
            __constructing = true
        }
        function __startProject() {
            renderTarget.currentTimeIndex = 0
            projectDocument.refresh()
            projectDocument.rewind()
            renderTarget.render()
            standbyRenderTimer.start()
            __constructing = false
        }
        function __handleWillLoad(numEstimated) {
            progressBar.visible = true
            progressBar.maximumValue = numEstimated
        }
        function __handleBeLoading(numLoaded, numEstimated) {
            progressBar.value = numLoaded
            progressBar.maximumValue = numEstimated
        }
        function __handleDidLoad(numLoaded, numEstimated) {
            progressBar.value = numLoaded
            progressBar.maximumValue = numEstimated
            progressBar.visible = false
        }
        onProjectWillCreate: __stopProject()
        onProjectDidCreate: __startProject()
        onProjectWillLoad: __stopProject()
        onProjectDidLoad: __startProject()
        onProjectWillSave: __stopProject()
        onProjectDidSave: __startProject()
        onCurrentModelChanged: {
            var currentModel = projectDocument.currentModel
            infoPanel.currentModelName = currentModel ? currentModel.name : infoPanel.defaultNullModelName
        }
        onCurrentMotionChanged: {
            renderTargetAnimation.setRange(0, projectDocument.durationTimeIndex)
            seek(currentTimeIndex)
            if (!__constructing) {
                renderTarget.render()
            }
        }
        onModelWillLoad: {
            model.modelWillLoad.connect(__handleWillLoad)
            model.modelBeLoading.connect(__handleBeLoading)
            model.modelDidLoad.connect(__handleDidLoad)
        }
        onModelDidLoad: {
            model.modelWillLoad.disconnect(__handleWillLoad)
            model.modelBeLoading.disconnect(__handleBeLoading)
            model.modelDidLoad.disconnect(__handleDidLoad)
            if (!skipConfirm) {
                if (confirmWindowLoader.status === Loader.Ready) {
                    var item = confirmWindowLoader.item
                    item.modelSource = model
                    item.show()
                }
                else {
                    confirmWindowLoader.setSource("ConfirmWindow.qml", { "modelSource": model })
                }
            }
            else if (!__constructing) {
                projectDocument.addModel(model, true)
            }
        }
        onMotionWillLoad: {
            motion.motionWillLoad.connect(__handleWillLoad)
            motion.motionBeLoading.connect(__handleBeLoading)
            motion.motionDidLoad.connect(__handleDidLoad)
        }
        onMotionDidLoad: {
            currentMotion = motion;
            motion.motionWillLoad.disconnect(__handleWillLoad)
            motion.motionBeLoading.disconnect(__handleBeLoading)
            motion.motionDidLoad.disconnect(__handleDidLoad)
            if (!__constructing) {
                renderTarget.currentTimeIndex = 0
                rewind()
                renderTarget.render()
            }
        }
        onPoseDidLoad: {
            seek(currentTimeIndex)
            renderTarget.render()
        }
    }
    VPVM.RenderTarget {
        id: renderTarget
        readonly property rect defaultViewportSetting: Qt.rect(scene.x, scene.offsetY, scene.width, scene.height)
        property real sceneFPS : 60
        function __handleTargetBonesDidBeginTransform() {
            renderTarget.playing = true
        }
        function __handleTargetBonesDidCommitTransform() {
            renderTarget.playing = false
            var bones = currentModel.targetBones, timeIndex = projectDocument.currentTimeIndex
            for (var i in bones) {
                var bone = bones[i]
                currentMotion.updateKeyframe(bone, timeIndex)
            }
        }
        function __handleTransformTypeChanged() {
            var type = scene.currentModel.transformType
            transformMode.mode = type
            boneTransformTypeDidChange(type)
        }
        function __handleBoneDidSelect(bone) {
            transformHandleSet.toggle(bone)
            infoPanel.currentBoneName = bone ? bone.name : infoPanel.defaultNullBoneName
            boneDidSelect(bone)
        }
        function __handleMorphWeightChanged() {
            if (standbyRenderTimer.running) {
                morphSlider.value = currentModel.firstTargetMorph.weight
            }
        }
        function __handleMorphDidSelect(morph) {
            if (morph) {
                morph.weightChanged.connect(__handleMorphWeightChanged);
                morph.sync()
                infoPanel.currentMorphName = morph.name
            }
            else {
                infoPanel.currentMorphName = infoPanel.defaultNullMorphlName
            }
            morphDidSelect(morph)
        }
        project: projectDocument
        viewport: defaultViewportSetting
        width: viewport.width
        height: viewport.height
        onCurrentTimeIndexChanged: {
            var timeIndex = currentTimeIndex
            projectDocument.seek(timeIndex);
            currentTimeIndexDidChange(timeIndex)
        }
        onCurrentFPSChanged: fpsCountPanel.value = currentFPS > 0 ? currentFPS : "N/A"
        onLastTimeIndexChanged: renderTargetAnimation.setRange(lastTimeIndex, projectDocument.durationTimeIndex)
        onModelDidUpload: {
            model.targetBonesDidBeginTransform.connect(__handleTargetBonesDidBeginTransform)
            model.targetBonesDidCommitTransform.connect(__handleTargetBonesDidCommitTransform)
            model.transformTypeChanged.connect(__handleTransformTypeChanged)
            model.boneDidSelect.connect(__handleBoneDidSelect)
            model.morphDidSelect.connect(__handleMorphDidSelect)
            model.selectOpaqueObject(model.availableBones[0])
            progressBar.visible = false
            progressBar.indeterminate = false
            scene.modelDidUpload(model)
        }
        onEncodeDidBegin: {
            scene.state = "encode"
        }
        onEncodeDidProceed: encodingPanel.text = qsTr("Encoding %1 of %2 frames").arg(proceed).arg(estimated)
        onEncodeDidFinish: {
            scene.state = "stop"
        }
        ProgressBar {
            id: progressBar
            visible: false
            anchors.centerIn: parent
        }
        Timer {
            id: standbyRenderTimer
            interval: 1000.0 / 60.0
            repeat: true
            running: true
            triggeredOnStart: true
            onTriggered: renderTarget.update()
        }
        NumberAnimation on currentTimeIndex {
            id: renderTargetAnimation
            from: 0
            running: false
            onRunningChanged: renderTarget.playing = running
            onStopped: {
                var s = scene.state
                if (s !== "stop" && s !== "pause") {
                    scene.state = "stop"
                    if (scene.loop) {
                        scene.state = "play"
                    }
                }
            }
            function setRange(from, to) {
                if (to <= 0) {
                    to = scene.project.durationTimeIndex
                }
                if (from >= 0 && to > 0 && to >= from) {
                    renderTargetAnimation.from = from;
                    renderTargetAnimation.to = to;
                    renderTargetAnimation.duration = projectDocument.millisecondsFromTimeIndex(to - from)
                }
            }
        }
        Keys.onPressed: {
            var keycode2closure = {};
            keycode2closure[Qt.Key_Plus] = function(event) { camera.zoom(-1) }
            keycode2closure[Qt.Key_Minus] = function(event) { camera.zoom(1) }
            keycode2closure[Qt.Key_L] = function(event) { transformMode.state = "local" }
            keycode2closure[Qt.Key_G] = function(event) { transformMode.state = "global" }
            keycode2closure[Qt.Key_V] = function(event) { transformMode.state = "view" }
            keycode2closure[Qt.Key_P] = function(event) { renderTarget.grabScreenImage() }
            keycode2closure[Qt.Key_Up] = function(event) {
                if (event.modifiers & Qt.ShiftModifier) {
                    camera.translate(0, 1)
                }
                else {
                    camera.rotate(0, 1)
                }
            }
            keycode2closure[Qt.Key_Down] = function(event) {
                if (event.modifiers & Qt.ShiftModifier) {
                    camera.translate(0, -1)
                }
                else {
                    camera.rotate(0, -1)
                }
            }
            keycode2closure[Qt.Key_Left] = function(event) {
                if (event.modifiers & Qt.ShiftModifier) {
                    camera.translate(-1, 0)
                }
                else {
                    camera.rotate(-1, 0)
                }
            }
            keycode2closure[Qt.Key_Right] = function(event) {
                if (event.modifiers & Qt.ShiftModifier) {
                    camera.translate(1, 0)
                }
                else {
                    camera.rotate(1, 0)
                }
            }
            var closure = keycode2closure[event.key]
            if (closure) {
                closure(event)
            }
        }
    }
    MouseArea {
        id: renderTargetMouseArea
        property int lastX : 0
        property int lastY : 0
        readonly property real wheelFactor : 0.05
        anchors.fill: renderTarget
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        onPressed: {
            var x = mouse.x, y = mouse.y
            lastX = x
            lastY = y
            renderTarget.forceActiveFocus()
            renderTarget.handleMousePress(x, y)
        }
        onPositionChanged: {
            var x = mouse.x, y = mouse.y
            renderTarget.handleMouseMove(x, y)
            if (pressed && !renderTarget.grabbingGizmo) {
                camera.rotate(x - lastX, y - lastY)
                lastX = x
                lastY = y
            }
        }
        onReleased: renderTarget.handleMouseRelease(mouse.x, mouse.y)
        onClicked: projectDocument.ray(mouse.x, mouse.y, width, height)
        onWheel: {
            if (wheel.modifiers & Qt.ShiftModifier) {
                var delta = wheel.pixelDelta
                camera.translate(delta.x * wheelFactor, delta.y * wheelFactor)
            }
            else {
                camera.zoom(wheel.pixelDelta.y * wheelFactor)
            }
        }
        states: State {
            when: projectDocument.currentModel.moving || cameraHandleSet.translating
            PropertyChanges {
                target: renderTargetMouseArea
                cursorShape: Qt.SizeVerCursor
            }
        }
    }
    /* left-top */
    InfoPanel {
        id: infoPanel
        fontPointSize: baseFontPointSize
        anchors { top: renderTarget.top; left: renderTarget.left; margins: scene.__cornerMarginSize }
        visible: isHUDAvailable
    }
    Text {
        id: playingPanel
        anchors { top: renderTarget.top; left: renderTarget.left; margins: scene.__cornerMarginSize }
        font { family: applicationPreference.fontFamily; pointSize: baseFontPointSize }
        color: "red"
        text: qsTr("Playing %1 of %2 frames").arg(Math.floor(renderTarget.currentTimeIndex)).arg(projectDocument.durationTimeIndex)
        visible: scene.playing
    }
    Text {
        id: encodingPanel
        anchors { top: renderTarget.top; left: renderTarget.left; margins: scene.__cornerMarginSize }
        font { family: applicationPreference.fontFamily; pointSize: baseFontPointSize }
        color: "red"
        text: qsTr("Encoding...")
        visible: scene.encoding
    }
    /* right-top */
    CameraHandleSet {
        id: cameraHandleSet
        anchors { top: renderTarget.top; right: renderTarget.right; margins: scene.__cornerMarginSize }
        iconPointSize: baseIconPointSize
        visible: isHUDAvailable
    }
    /* left-bottom */
    Column {
        id: preference
        anchors { left: renderTarget.left; bottom: renderTarget.bottom; margins: scene.__cornerMarginSize }
        FPSCountPanel { id: fpsCountPanel; fontPointSize: baseFontPointSize }
        Row {
            visible: isHUDAvailable
            Text {
                id: toggleTimeline
                font { family: transformHandleSet.fontFamilyName; pointSize: baseIconPointSize }
                color: "gray"
                text: FontAwesome.Icon.Columns
                MouseArea {
                    anchors.fill: parent
                    onClicked: (mouse.modifiers & Qt.ControlModifier) ? toggleTimelineWindowed() : toggleTimelineVisible()
                    onPressAndHold: toggleTimelineWindowed()
                }
            }
        }
    }
    /* right-bottom */
    Slider {
        id: morphSlider
        anchors { right: renderTarget.right; bottom: renderTarget.bottom; margins: scene.__cornerMarginSize }
        visible: isHUDAvailable
        width: transformHandleSet.width
        enabled: projectDocument.currentModel && projectDocument.currentModel.firstTargetMorph
        minimumValue: 0.0
        maximumValue: 1.0
        State {
            name: "valueChange"
            when: parent.enabled
            PropertyChanges {
                target: morphSlider
                value: projectDocument.currentModel.firstTargetMorph.weight
            }
        }
        onPressedChanged: {
            var motion = projectDocument.currentMotion
            if (motion && !pressed) {
                var morph = projectDocument.currentModel.firstTargetMorph
                var timeIndex = projectDocument.currentTimeIndex
                motion.updateKeyframe(morph, timeIndex)
            }
        }
        onValueChanged: {
            projectDocument.currentModel.firstTargetMorph.weight = value
            renderTarget.render()
        }
    }
    TransformHandleSet {
        id: transformHandleSet
        anchors { right: renderTarget.right; bottom: morphSlider.top; margins: scene.__cornerMarginSize }
        iconPointSize: baseIconPointSize
        visible: isHUDAvailable
        onAxisTypeSet: projectDocument.currentModel.axisType = value
        onBeginTranslate: projectDocument.currentModel.beginTranslate(delta)
        onTranslate: projectDocument.currentModel.translate(delta)
        onEndTranslate: projectDocument.currentModel.endTranslate()
        onBeginRotate: projectDocument.currentModel.beginRotate(delta)
        onRotate:  projectDocument.currentModel.rotate(delta)
        onEndRotate: projectDocument.currentModel.endRotate()
    }
    TransformMode {
        id: transformMode
        anchors { horizontalCenter: transformHandleSet.horizontalCenter; bottom: transformHandleSet.top }
        enabled: projectDocument.currentModel
        visible: isHUDAvailable
        onModeChanged: if (projectDocument.currentModel) projectDocument.currentModel.transformType = mode
    }
}
