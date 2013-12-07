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

ApplicationWindow {
    id: confirmWindow
    property var modelSource: null
    signal accept()
    signal reject()
    title: qsTr("Confirm of loading ") + (modelSource ? modelSource.name : "")
    width: 640
    height: 480
    minimumWidth: 480
    minimumHeight: 320
    modality: Qt.ApplicationModal
    onClosing: {
        modelSource.release()
        close.accepted = true
        reject()
    }
    function makeLink(comment) {
        var newComment = comment
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/&/g, "&amp;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#39;")
            .replace(/\r\n/g, "<br>")
            .replace(/\n/g, "<br>")
            .replace(/@(\w+)/g, "<a href='https://twitter.com/$1'>@$1</a>")
        return newComment
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        ComboBox {
            anchors.horizontalCenter: parent.horizontalCenter
            model: [
                { "text": qsTr("Japanese"), "value": VPVM.Project.Japanese },
                { "text": qsTr("English"), "value": VPVM.Project.English }
            ]
            onCurrentIndexChanged: {
                var item = model[currentIndex]
                modelSource.language = item.value
            }
        }
        TextArea {
            id: confirmText
            Layout.fillWidth: true
            Layout.fillHeight: true
            font.family: applicationPreference.fontFamily
            text: modelSource ? makeLink(modelSource.comment) : ""
            textFormat: TextEdit.RichText
            readOnly: true
            onLinkActivated: Qt.openUrlExternally(link)
        }
        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: qsTr("Cancel")
                isDefault: true
                onClicked: {
                    modelSource.release()
                    confirmWindow.close()
                    reject()
                }
            }
            Button {
                text: qsTr("Accept")
                onClicked: {
                    project.addModel(modelSource)
                    confirmWindow.close()
                    accept()
                }
            }
        }
    }
}
