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
import "FontAwesome.js" as FontAwesome

Text {
    id: rotationButton
    property int axisType
    property string axisColor
    signal axisTypeSet(int value)
    signal beginRotate(real delta)
    signal rotate(real delta)
    signal endRotate()
    text: FontAwesome.Icon.Refresh
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPressed: {
            rotationButton.axisTypeSet(axisType)
            rotationButton.beginRotate(mouse.y * (mouse.modifiers & Qt.AltModifier ? -1 : 1))
        }
        onMouseYChanged: rotationButton.rotate(mouse.y * (mouse.modifiers & Qt.AltModifier ? -1 : 1))
        onReleased: rotationButton.endRotate()
        onEntered: cursorShape = Qt.SizeVerCursor
    }
    state: "disabled"
    states: [
        State {
            name: "enabled"
            PropertyChanges { target: rotationButton; color: rotationButton.axisColor; enabled: true }
        },
        State {
            name: "disabled"
            PropertyChanges { target: rotationButton; color: "lightgray"; enabled: false }
        }
    ]
}
