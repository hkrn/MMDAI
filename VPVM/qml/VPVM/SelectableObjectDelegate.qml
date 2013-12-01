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
import com.github.mmdai.VPVM 1.0
import "FontAwesome.js" as FontAwesome

Item {
    id: selectableObjectDelegate
    property Component delegateComponent : selectableObjectDelegateComponent
    property string fontAwesome
    property Project project
    signal objectSelected(var uuid, string name)
    Component {
        id: selectableObjectDelegateComponent
        Rectangle {
            id: selectableObjectDelegateRectangle
            width: selectableObjectDelegate.width
            height: collapsed ? 0 : childrenRect.height
            Text {
                id: iconLabel
                anchors { left: parent.left; verticalCenter: nameLabel.verticalCenter; margins: 2 }
                font { family: fontAwesome; pixelSize: 18 }
                color: "black"
                visible: !collapsed
                text: hasKeyframe ? FontAwesome.Icon.Check : FontAwesome.Icon.CheckEmpty
            }
            Text {
                id: nameLabel
                anchors { left: iconLabel.right }
                font { family: applicationPreference.fontFamily; pixelSize: 18 }
                color: "black"
                visible: !collapsed
                text: name
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectableObjectDelegate.objectSelected(uuid, name)
                }
            }
        }
    }
}
