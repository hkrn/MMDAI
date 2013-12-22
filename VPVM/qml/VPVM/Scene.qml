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
import com.github.mmdai.VPVM 1.0 as VPVM
import "FontAwesome.js" as FontAwesome

Item {
    id: scene
    readonly property alias project: projectDocument
    readonly property alias camera : projectDocument.camera
    readonly property alias light  : projectDocument.light
    readonly property alias viewport : renderTarget.viewport
    readonly property alias graphicsDevice : renderTarget.graphicsDevice
    readonly property alias canSetRange : audioEngine.seekable
    readonly property bool hasBoneSelected : projectDocument.currentModel && projectDocument.currentModel.firstTargetBone
    readonly property bool hasMorphSelected : projectDocument.currentModel && projectDocument.currentModel.firstTargetMorph
    readonly property bool playing: state === "play" || state === "export"
    readonly property bool encoding: state === "encode"
    readonly property bool __tryStopping : state !== "stop" && state !== "pause" && !audioEngine.stopping
    readonly property int __cornerMarginSize : 5
    property alias loop : projectDocument.loop
    property alias currentModel : projectDocument.currentModel
    property alias currentMotion : projectDocument.currentMotion
    property alias enableSnapGizmo : renderTarget.enableSnapGizmo
    property alias snapGizmoStepSize : renderTarget.snapGizmoStepSize
    property alias shadowMapSize : renderTarget.shadowMapSize
    property alias editMode : renderTarget.editMode
    property int baseFontPointSize : 16
    property int baseIconPointSize : 48
    property bool isHUDAvailable : true
    property real offsetY : 0
    property string lastState: "stop"
    property var __keycode2closures : ({})
    signal toggleTimelineVisible()
    signal toggleTimelineWindowed()
    signal uploadingModelDidSucceed(var model, var isProject)
    signal uploadingModelDidFail(var model, var isProject)
    signal uploadingEffectDidSucceed(var model)
    signal uploadingEffectDidFail(var model)
    signal encodeDidFinish(var isNormalExit)
    signal encodeDidCancel()
    signal notificationDidPost(string message)
    signal currentTimeIndexDidChange(int timeIndex)
    signal boneTransformTypeDidChange(int type)
    signal boneDidSelect(var bone)
    signal morphDidSelect(var morph)

    Component.onCompleted: {
        __keycode2closures[Qt.Key_Plus] = function(event) { camera.zoom(-1) }
        __keycode2closures[Qt.Key_Minus] = function(event) { camera.zoom(1) }
        __keycode2closures[Qt.Key_L] = function(event) { transformMode.state = "local" }
        __keycode2closures[Qt.Key_G] = function(event) { transformMode.state = "global" }
        __keycode2closures[Qt.Key_V] = function(event) { transformMode.state = "view" }
        __keycode2closures[Qt.Key_P] = function(event) { renderTarget.grabScreenImage() }
        __keycode2closures[Qt.Key_Up] = function(event) {
            if (event.modifiers & Qt.ShiftModifier) {
                camera.translate(0, 1)
            }
            else {
                camera.rotate(0, 1)
            }
        }
        __keycode2closures[Qt.Key_Down] = function(event) {
            if (event.modifiers & Qt.ShiftModifier) {
                camera.translate(0, -1)
            }
            else {
                camera.rotate(0, -1)
            }
        }
        __keycode2closures[Qt.Key_Left] = function(event) {
            if (event.modifiers & Qt.ShiftModifier) {
                camera.translate(-1, 0)
            }
            else {
                camera.rotate(-1, 0)
            }
        }
        __keycode2closures[Qt.Key_Right] = function(event) {
            if (event.modifiers & Qt.ShiftModifier) {
                camera.translate(1, 0)
            }
            else {
                camera.rotate(1, 0)
            }
        }
        if (Qt.platform.os !== "osx") {
            baseFontPointSize = 12
            baseIconPointSize = 36
        }
    }

    Loader {
        id: confirmWindowLoader
        // asynchronous: true
        visible: status === Loader.Ready
        function __handleAccept() {
            buzyIndicator.running = true
            renderTarget.forceActiveFocus()
        }
        function __handleReject() {
            buzyIndicator.running = false
            renderTarget.forceActiveFocus()
        }
        onLoaded: {
            item.accept.connect(__handleAccept)
            item.reject.connect(__handleReject)
            item.show()
        }
    }
    VPVM.ALAudioEngine {
        id: audioEngine
        property bool stopping : false
        function tryStop() {
            if (__tryStopping) {
                stopping = true
                scene.state = "stop"
                if (scene.loop) {
                    scene.state = "play"
                }
            }
        }
        onPlayingDidPerform: renderTarget.playing = true
        onStoppingDidPerform: tryStop()
        onPlayingNotPerformed: renderTargetAnimation.start()
        onStoppingNotPerformed: tryStop()
        onAudioSourceDidLoad: notificationDidPost(qsTr("The audio file was loaded normally."))
        onSourceChanged: project.audioSource = source
        onErrorDidHappen: notificationDidPost(qsTr("Could not load the audio file. For more verbose reason, see log."))
        onTimeIndexChanged: renderTarget.currentTimeIndex = timeIndex
    }

    function seek(timeIndex) {
        projectDocument.seek(timeIndex)
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
    function loadAudio(fileUrl) {
        VPVM.ALAudioContext.initialize()
        audioEngine.source = fileUrl
    }
    function loadVideo(fileUrl) {
        renderTarget.videoUrl = fileUrl
    }
    function exportImage(fileUrl, size) {
        if (fileUrl.toString() !== "") {
            renderTarget.exportImage(fileUrl, size)
        }
    }
    function exportVideo(fileUrl, size, videoType, frameImageType) {
        state = "export"
        if (fileUrl.toString() !== "") {
            renderTarget.exportVideo(fileUrl, size, videoType, frameImageType)
        }
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
                    audioEngine.play()
                    standbyRenderTimer.stop()
                }
            }
        },
        State {
            name: "export"
            PropertyChanges { target: scene; isHUDAvailable: false }
            StateChangeScript { script: standbyRenderTimer.stop() }
        },
        State {
            name: "pause"
            PropertyChanges { target: scene; isHUDAvailable: true }
            StateChangeScript {
                script: {
                    audioEngine.stop()
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
                    audioEngine.stop()
                    renderTargetAnimation.stop()
                    projectDocument.rewind()
                    renderTarget.render()
                    standbyRenderTimer.start()
                    audioEngine.stopping = false
                }
            }
        },
        State {
            name: "encode"
            PropertyChanges { target: scene; isHUDAvailable: false }
            StateChangeScript {
                script: {
                    renderTargetAnimation.stop()
                    projectDocument.rewind()
                    renderTarget.render()
                }
            }
        },
        State {
            name: "suspend"
            PropertyChanges { target: scene; isHUDAvailable: false }
            StateChangeScript {
                script: {
                    renderTargetAnimation.stop()
                    standbyRenderTimer.stop()
                    lastState = state
                }
            }
        }
    ]

    VPVM.Project {
        id: projectDocument
        property bool __constructing: false
        function __stopProject() {
            standbyRenderTimer.stop()
            renderTarget.toggleRunning(false)
            __constructing = true
        }
        function __startProject() {
            renderTarget.currentTimeIndex = 0
            projectDocument.refresh()
            projectDocument.rewind()
            var audioSource = projectDocument.audioSource
            if (audioSource.toString() !== "") {
                VPVM.ALAudioContext.initialize()
                audioEngine.source = audioSource
            }
            renderTarget.toggleRunning(true)
            standbyRenderTimer.start()
            __constructing = false
        }
        function __handleWillLoad(numEstimated) {
            buzyIndicator.running = true
        }
        function __handleBeLoading(numLoaded, numEstimated) {
        }
        function __handleDidLoad(numLoaded, numEstimated) {
            buzyIndicator.running = false
        }
        camera.seekable: playing || currentMotion === camera.motion
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
                projectDocument.addModel(model)
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
            renderTarget.render()
        }
        function __handleMorphDidSelect(morph) {
            if (morph) {
                morph.weightChanged.connect(__handleMorphWeightChanged)
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
        onInitializedChanged: {
            if (initialized && applicationBootstrapOption.hasJson) {
                renderTarget.loadJson(applicationBootstrapOption.json)
            }
        }
        onCurrentTimeIndexChanged: {
            var timeIndex = currentTimeIndex
            projectDocument.seek(timeIndex);
            currentTimeIndexDidChange(timeIndex)
        }
        onCurrentFPSChanged: fpsCountPanel.value = currentFPS > 0 ? currentFPS : "N/A"
        onLastTimeIndexChanged: renderTargetAnimation.setRange(lastTimeIndex, projectDocument.durationTimeIndex)
        onUploadingModelDidSucceed: {
            model.targetBonesDidBeginTransform.connect(__handleTargetBonesDidBeginTransform)
            model.targetBonesDidCommitTransform.connect(__handleTargetBonesDidCommitTransform)
            model.transformTypeChanged.connect(__handleTransformTypeChanged)
            model.boneDidSelect.connect(__handleBoneDidSelect)
            model.morphDidSelect.connect(__handleMorphDidSelect)
            if (model.availableBones.length > 0) {
                model.selectOpaqueObject(model.availableBones[0])
            }
            buzyIndicator.running = false
            scene.uploadingModelDidSucceed(model, isProject)
        }
        onUploadingModelDidFail: {
            if (model === currentModel) {
                currentModel = null
            }
            buzyIndicator.running = false
            scene.uploadingModelDidFail(model, isProject)
        }
        onUploadingEffectDidSucceed: scene.uploadingEffectDidSucceed(model)
        onUploadingEffectDidFail: scene.uploadingEffectDidFail(model)
        onErrorDidHappen: notificationDidPost(message)
        onEncodeDidBegin: scene.state = "encode"
        onEncodeDidProceed: encodingPanel.text = qsTr("Encoding %1 of %2 frames").arg(proceed).arg(estimated)
        onEncodeDidCancel: scene.encodeDidCancel()
        onEncodeDidFinish: {
            scene.state = "stop"
            scene.encodeDidFinish(isNormalExit)
        }
        BusyIndicator {
            id: buzyIndicator
            running: false
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
            onStopped: audioEngine.tryStop()
            function setRange(from, to) {
                if (to <= 0) {
                    to = scene.project.durationTimeIndex
                }
                if (from >= 0 && to >= from) {
                    renderTargetAnimation.from = from;
                    renderTargetAnimation.to = to;
                    renderTargetAnimation.duration = projectDocument.millisecondsFromTimeIndex(to - from)
                }
            }
        }
        Keys.onPressed: {
            var closure = __keycode2closures[event.key]
            if (closure) {
                closure(event)
            }
        }
        DropArea {
            anchors.fill: parent
            onDropped: {
                if (drop.hasUrls) {
                    for (var i in drop.urls) {
                        var url = drop.urls[i]
                        project.loadModel(url)
                    }
                }
            }
        }
    }
    Keys.onPressed: {
        event.accepted = renderTarget.handleKeyPress(event.key, event.modifiers)
    }
    MouseArea {
        id: renderTargetMouseArea
        property int lastX : 0
        property int lastY : 0
        readonly property real wheelFactor : 0.05
        anchors.fill: renderTarget
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: !playing
        hoverEnabled: true
        onPressed: {
            var x = mouse.x, y = mouse.y
            lastX = x
            lastY = y
            renderTarget.forceActiveFocus()
            renderTarget.handleMousePress(x, y, mouse.button)
        }
        onPositionChanged: {
            var x = mouse.x, y = mouse.y
            if (!renderTarget.handleMouseMove(x, y, mouse.button) && pressed) {
                camera.rotate(x - lastX, y - lastY)
                lastX = x
                lastY = y
            }
        }
        onReleased: renderTarget.handleMouseRelease(mouse.x, mouse.y, mouse.button)
        onClicked: projectDocument.ray(mouse.x, mouse.y, width, height)
        onWheel: {
            var delta = wheel.pixelDelta
            if (!renderTarget.handleMouseWheel(delta.x, delta.y)) {
                if (wheel.modifiers & Qt.ShiftModifier) {
                    camera.translate(delta.x * wheelFactor, delta.y * wheelFactor)
                }
                else {
                    camera.zoom(delta.y * wheelFactor)
                }
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
    TransformHandleSet {
        id: transformHandleSet
        anchors { right: renderTarget.right; bottom: renderTarget.bottom; margins: scene.__cornerMarginSize }
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
