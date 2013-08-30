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
    id: projectPreferenceDialog
    title: qsTr("Project Preference")
    color: systemPalette.window
    visible: false
    width: 600
    height: 480
    ColumnLayout {
        anchors.fill: parent
        TabView {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
            anchors.fill: parent
            anchors.margins: 10
            Tab {
                title: qsTr("Physics")
                anchors.margins: tabView.anchors.margins
                ColumnLayout {
                    AxesSpinBox {
                        Layout.columnSpan: 2
                        title: qsTr("Gravity and Direction")
                    }
                    GridLayout {
                        columns: 2
                        Label { text: qsTr("Seed") }
                        SpinBox {}
                        Label { text: qsTr("SubStep") }
                        SpinBox { value: 2 }
                    }
                    Rectangle { Layout.fillHeight: true; Layout.columnSpan: 2 }
                }
            }
            Tab {
                title: qsTr("Render Order")
                anchors.margins: tabView.anchors.margins
                ColumnLayout {
                    TableView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        headerVisible: false
                        model: ListModel {
                            Component.onCompleted: {
                                for (var i = 0; i < 50; i++) {
                                    append({ name: "model" + i })
                                }
                            }
                        }
                        TableViewColumn { role: "name"; title: "name" }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        Button {
                            text: qsTr("Up")
                        }
                        Button {
                            text: qsTr("Down")
                        }
                        Button {
                            text: qsTr("Reset")
                        }
                    }
                }
            }
            Tab {
                title: qsTr("Acceleration")
                anchors.margins: tabView.anchors.margins
                ColumnLayout {
                    ExclusiveGroup { id: accelerationGroup }
                    RadioButton {
                        text: qsTr("None")
                        exclusiveGroup: accelerationGroup
                        checked: true
                    }
                    Label {
                        text: "Disable Any Acceleration. This is slow but stable.";
                        wrapMode: Text.WordWrap
                    }
                    RadioButton {
                        text: qsTr("Parallel")
                        exclusiveGroup: accelerationGroup
                    }
                    Label {
                        text: "Enable Parallel Mode. This is fast but may cause unstable."
                        wrapMode: Text.WordWrap
                    }
                    RadioButton {
                        text: qsTr("OpenCL (CPU)")
                        exclusiveGroup: accelerationGroup
                    }
                    Label {
                        text: "Enable OpenCL (CPU) Mode. This is fast but may cause unstable."
                        wrapMode: Text.WordWrap
                    }
                    RadioButton {
                        text: qsTr("OpenCL (GPU)")
                        exclusiveGroup: accelerationGroup
                    }
                    Label {
                        text: "Enable OpenCL (GPU) Mode. This is very fast but may cause unstable."
                        wrapMode: Text.WordWrap
                    }
                    Rectangle { Layout.fillHeight: true }
                }
            }
        }
        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            Layout.fillWidth: true
            Button {
                text: qsTr("Apply")
                isDefault: true
                onClicked: {
                }
            }
            Button {
                text: qsTr("Cancel")
                onClicked: {
                }
            }
            Button {
                text: qsTr("OK")
                onClicked: {
                }
            }
        }
    }
}
