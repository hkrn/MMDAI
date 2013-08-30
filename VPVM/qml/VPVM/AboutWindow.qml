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
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: aboutWindow
    title: openAboutAction.text
    modality: Qt.WindowModal
    width: 600
    height: 400
    ListModel {
        id: librariesModel
        ListElement { name: "libvpvl2"; license: "3-Clauses BSD"; url: "https://github.com/hkrn/MMDAI/" }
        ListElement { name: "MMDAgent"; license: "3-Clauses BSD"; url: "https://sf.net/projects/MMDAgent/" }
        ListElement { name: "Bullet Physics"; license: "zlib"; url: "http://bulletphysics.org" }
        ListElement { name: "Open Asset Import Library"; license: "3-Clauses BSD"; url: "http://assimp.sf.net" }
        ListElement { name: "zlib"; license: "zlib"; url: "http://zlib.net" }
        ListElement { name: "minizip"; license: "zlib"; url: "http://www.winimage.com/zLibDll/minizip.html" }
        ListElement { name: "Threading Building Blocks"; license: "GPL with link exception"; url: "http://threadingbuildingblocks.org" }
        ListElement { name: "ICU"; license: "MIT"; url: "http://icu-project.org" }
        ListElement { name: "GLM"; license: "MIT"; url: "http://glm.g-truc.net" }
        ListElement { name: "OpenAL Soft"; license: "LGPL"; url: "http://kcat.strangesoft.net/openal.html" }
        ListElement { name: "ALURE"; license: "MIT"; url: "http://kcat.strangesoft.net/alure.html" }
        ListElement { name: "glog"; license: "3-Clauses BSD"; url: "https://code.google.com/p/google-glog/" }
        ListElement { name: "libgizmo"; license: "MIT"; url: "https://github.com/hkrn/LibGizmo/" }
        ListElement { name: "nvFX"; license: "2-Clauses BSD"; url: "https://github.com/tlorach/nvFX/" }
        ListElement { name: "Regal"; license: "2-Clauses BSD"; url: "https://github.com/p3/regal/" }
    }
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        ColumnLayout {
            GridLayout {
                Layout.alignment: Qt.AlignCenter
                columns: 2
                Text { text: qsTr("name") }
                Text { text: Qt.application.name }
                Text { text: qsTr("version") }
                Text { text: Qt.application.version }
                Text { text: qsTr("argument") }
                Text { text: (Qt.application.argument || []).join(" ") }
            }
            Text { text: qsTr("%1 is an open source software and uses below open source softwares.").arg(Qt.application.name) }
            TableView {
                id: licenseTable
                Layout.fillWidth: true
                Layout.fillHeight: true
                TableViewColumn { role: "name"; title: "Name" }
                TableViewColumn { role: "license"; title: "License" }
                TableViewColumn { role: "url"; title: "URL" }
                model: librariesModel
            }
            Button {
                Layout.alignment: Qt.AlignCenter
                anchors.horizontalCenter: parent.horizontalCenter
                isDefault: true
                text: qsTr("OK")
                onClicked: aboutWindow.close()
            }
        }
    }
}
