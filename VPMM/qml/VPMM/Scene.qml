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
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

Item {
    id: scene
    readonly property alias project: projectDocument
    readonly property alias camera : projectDocument.camera
    readonly property alias light  : projectDocument.light
    readonly property alias importer : modelImporter
    readonly property alias viewport : renderTarget.viewport
    property alias currentModel : projectDocument.currentModel
    property real offsetX: 0
    property real offsetY: 0
    property string lastStateAtSuspend: "pause"
    property var __keycode2closures : ({})

    signal notificationDidPost(string message)
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

    VPMM.Project {
        id: projectDocument
        onModelDidLoad: {
            projectDocument.addModel(model)
        }
        onModelDidFailLoading: {
            busyIndicator.running = false
            notificationDidPost(qsTr("The model cannot be loaded: %1").arg(project.errorString))
        }
        onMotionDidFailLoading: {
            notificationDidPost(qsTr("The motion cannot be loaded: %1").arg(project.errorString))
        }
        onMotionDidLoad: {
            currentMotion = motion;
            renderTarget.currentTimeIndex = 0
            rewind()
            renderTarget.render()
        }
        onPoseDidLoad: {
            seek(currentTimeIndex)
            renderTarget.render()
        }
    }
    VPMM.Importer {
        id: modelImporter
        project: projectDocument
    }
    VPMM.RenderTarget {
        id: renderTarget
        readonly property rect defaultViewportSetting: Qt.rect(scene.x + offsetX, scene.y + offsetY, scene.width, scene.height)
        function __handleMorphWeightChanged() {
            renderTarget.render()
        }
        function __handleMorphDidSelect(morph) {
            if (morph) {
                morph.weightChanged.connect(__handleMorphWeightChanged)
                morph.sync()
            }
            morphDidSelect(morph)
        }
        Layout.fillHeight: true
        project: projectDocument
        viewport: defaultViewportSetting
        width: viewport.width
        height: viewport.height
        onInitializedChanged: {
            if (initialized && applicationBootstrapOption.hasJson) {
                renderTarget.loadJson(applicationBootstrapOption.json)
            }
        }
        onUploadingModelDidSucceed: {
            model.morphDidSelect.connect(__handleMorphDidSelect)
        }
        Timer {
            id: standbyRenderTimer
            interval: 1000.0 / 60.0
            repeat: true
            running: true
            triggeredOnStart: true
            onTriggered: renderTarget.update()
        }
        Keys.onPressed: {
            var closure = __keycode2closures[event.key]
            if (closure) {
                closure(event)
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
    }
}
