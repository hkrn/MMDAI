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
    readonly property alias viewport : renderTarget.viewport
    property real offsetX: 0
    property real offsetY: 0
    property string lastStateAtSuspend: "stop"
    property var __keycode2closures : ({})
    signal notificationDidPost(string message)

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
            motion.motionWillLoad.disconnect(__handleWillLoad)
            motion.motionBeLoading.disconnect(__handleBeLoading)
            motion.motionDidLoad.disconnect(__handleDidLoad)
            renderTarget.currentTimeIndex = 0
            rewind()
            renderTarget.render()
        }
        onPoseDidLoad: {
            seek(currentTimeIndex)
            renderTarget.render()
        }
    }
    VPMM.RenderTarget {
        id: renderTarget
        readonly property rect defaultViewportSetting: Qt.rect(scene.x + offsetX, scene.y + offsetY, scene.width, scene.height)
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
