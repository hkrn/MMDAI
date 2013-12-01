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

Rectangle {
    id: notificationArea
    property string text
    property real duration : 3000
    property var notificationMessages: []
    color: "black"
    opacity: 0.75
    function notify(value) {
        text = value
        state = "show"
        notificationMessages.push(value)
    }
    Text {
        id: notificationText
        anchors.centerIn: parent
        font { family: applicationPreference.fontFamily; pointSize: 16; bold: true }
        text: notificationArea.text
        visible: parent.state === "show"
        color: "white"
    }
    state: "hide"
    states: [
        State {
            name: "show"
            PropertyChanges { target: notificationArea; height: 25 }
        },
        State {
            name: "hide"
            PropertyChanges { target: notificationArea; height: 0 }
        }
    ]
    onStateChanged: {
        if (state === "show") {
            showTimer.start()
        }
    }
    transitions: Transition {
        NumberAnimation { properties: "height"; duration: 400 }
    }
    Timer {
        id: showTimer
        interval: notificationArea.duration
        onTriggered: notificationArea.state = "hide"
    }
}
