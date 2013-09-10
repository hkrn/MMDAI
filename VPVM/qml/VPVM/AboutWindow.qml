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
import com.github.mmdai.VPVM 1.0 as VPVM

ApplicationWindow {
    id: aboutWindow
    title: openAboutAction.text
    modality: Qt.WindowModal
    width: 700
    height: 500
    ListModel {
        id: licenseTableModel
        property int currentRow: 0
        ListElement { name: "libvpvl2"; display: "libvpvl2"; license: "3-Clauses BSD"; url: "https://github.com/hkrn/MMDAI/" }
        ListElement { name: "MMDAgent"; display: "MMDAgent"; license: "3-Clauses BSD"; url: "https://sf.net/projects/MMDAgent/" }
        ListElement { name: "bullet"; display: "Bullet Physics"; license: "zlib"; url: "http://bulletphysics.org" }
        ListElement { name: "assimp"; display: "assimp (Open Asset Import Library)"; license: "3-Clauses BSD"; url: "http://assimp.sf.net" }
        ListElement { name: "GLEW"; display: "GLEW (OpenGL Extension Wrangler)"; license: "3-Clauses BSD"; url: "http://glew.sf.net" }
        ListElement { name: "zlib"; display: "zlib"; license: "zlib"; url: "http://zlib.net" }
        ListElement { name: "minizip"; display: "minizip"; license: "zlib"; url: "http://www.winimage.com/zLibDll/minizip.html" }
        ListElement { name: "TBB"; display: "TBB (Threading Building Blocks)"; license: "GPL with link exception"; url: "http://threadingbuildingblocks.org" }
        ListElement { name: "ICU"; display: "ICU (International Components for Unicode)"; license: "MIT"; url: "http://icu-project.org" }
        ListElement { name: "GLM"; display: "GLM (OpenGL Mathematics)"; license: "MIT"; url: "http://glm.g-truc.net" }
        ListElement { name: "libav"; display: "libav"; license: "LGPL"; url: "http://libav.org" }
        ListElement { name: "ALsoft"; display: "OpenAL Soft"; license: "LGPL"; url: "http://kcat.strangesoft.net/openal.html" }
        ListElement { name: "ALURE"; display: "ALURE"; license: "MIT"; url: "http://kcat.strangesoft.net/alure.html" }
        ListElement { name: "glog"; display: "glog"; license: "3-Clauses BSD"; url: "https://code.google.com/p/google-glog/" }
        ListElement { name: "libgizmo"; display: "libgizmo"; license: "MIT"; url: "https://github.com/hkrn/LibGizmo/" }
        ListElement { name: "nvFX"; display: "nvFX"; license: "2-Clauses BSD"; url: "https://github.com/tlorach/nvFX/" }
        ListElement { name: "Regal"; display: "Regal"; license: "2-Clauses BSD"; url: "https://github.com/p3/regal/" }
        ListElement { name: "FontAwesome"; display: "Font Awesome"; license: ""; url: "https://github.com/fort-awesome/" }
    }
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        ColumnLayout {
            Label {
                Layout.fillWidth: true
                font.pointSize: 20
                text: "%1 (version=%2 arguments=%3)".arg(Qt.application.name).arg(Qt.application.version).arg((Qt.application.argument || []).join(" "))
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("%1 is an open source software that is distributed under 3-Clauses BSD license (same as libvpvl2) and %1 also uses below open source softwares and libraries.").arg(Qt.application.name)
                wrapMode: Text.WordWrap
            }
            TabView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Tab {
                    title: "List"
                    anchors.margins: 10
                    TableView {
                        id: licenseTable
                        sortIndicatorVisible: true
                        TableViewColumn { role: "display"; title: "Name"; width: 225 }
                        TableViewColumn { role: "license"; title: "License"; width: 125 }
                        TableViewColumn { role: "url"; title: "URL"; width: 300 }
                        model: licenseTableModel
                        onDoubleClicked: Qt.openUrlExternally(licenseTableModel.get(row).url)
                        onCurrentRowChanged: licenseTableModel.currentRow = currentRow
                    }
                }
                Tab {
                    id: licenseTextTab
                    title: qsTr("License")
                    anchors.margins: 10
                    TextArea {
                        id: licenseTextArea
                        font: { family: "sans-serif" }
                        readOnly: true
                        text: VPVM.UIAuxHelper.slurpLicenseText(licenseTableModel.get(licenseTableModel.currentRow).name)
                    }
                }
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
