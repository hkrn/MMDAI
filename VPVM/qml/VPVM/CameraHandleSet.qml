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
import "FontAwesome.js" as FontAwesome

Row {
    id: cameraHandleSet
    readonly property string fontFamilyName: "FontAwesome"
    readonly property color handleColor: "#ffdc00"
    readonly property bool translating: moveCamera.moving
    readonly property bool zooming: zoomCamera.moving
    property int iconPointSize : 48
    Text {
        property bool moving: false
        property int lastY: 0
        id: zoomCamera
        font { family: cameraHandleSet.fontFamilyName; pointSize: cameraHandleSet.iconPointSize }
        color: cameraHandleSet.handleColor
        text: FontAwesome.Icon.Search
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: cursorShape = Qt.SizeVerCursor
            onPressed: {
                zoomCamera.moving = true
                zoomCamera.lastY = mouse.y
            }
            onMouseYChanged: {
                if (zoomCamera.moving) {
                    var y = mouse.y
                    camera.zoom((y - zoomCamera.lastY) * (mouse.modifiers & Qt.AltModifier ? -1 : 1))
                    zoomCamera.lastY = y
                }
            }
            onReleased: zoomCamera.moving = false
        }
    }
    Text {
        property bool moving: false
        property int lastX: 0
        property int lastY: 0
        id: moveCamera
        font { family: cameraHandleSet.fontFamilyName; pointSize: cameraHandleSet.iconPointSize }
        color: cameraHandleSet.handleColor
        text: FontAwesome.Icon.Move
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: cursorShape = Qt.SizeVerCursor
            onPressed: {
                moveCamera.moving = true
                moveCamera.lastX = mouse.x
                moveCamera.lastY = mouse.y
            }
            onMouseXChanged: {
                if (moveCamera.moving) {
                    var x = mouse.x
                    camera.translate(-(x - moveCamera.lastX) * (mouse.modifiers & Qt.AltModifier ? -1 : 1), 0)
                    moveCamera.lastX = x
                }
            }
            onMouseYChanged: {
                if (moveCamera.moving) {
                    var y = mouse.y
                    camera.translate(0, (y - moveCamera.lastY) * (mouse.modifiers & Qt.AltModifier ? -1 : 1))
                    moveCamera.lastY = y
                }
            }
            onReleased: moveCamera.moving = false
        }
    }
}
