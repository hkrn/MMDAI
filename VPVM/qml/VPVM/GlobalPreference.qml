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
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPVM 1.0 as VPVM

ApplicationWindow {
    id: globalPreferenceDialog
    property var graphicsDevice
    title: qsTr("Global Preference")
    color: systemPalette.window
    width: 600
    height: 480
    ColumnLayout {
        id: preferenceLayout
        anchors.fill: parent
        anchors.margins: 10
        TabView {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
            anchors.margins: preferenceLayout.anchors.margins
            Tab {
                id: graphicsDeviceTab
                title: qsTr("Graphics Device")
                anchors.margins: tabView.anchors.margins
                GridLayout {
                    columns: 2
                    Layout.fillWidth: true
                    Label { text: qsTr("Version") }
                    TextField {
                        Layout.fillWidth: true
                        text: graphicsDevice.version
                        readOnly: true
                    }
                    Label { text: qsTr("Renderer") }
                    TextField {
                        Layout.fillWidth: true
                        text: graphicsDevice.renderer
                        readOnly: true
                    }
                    Label { text: qsTr("Vendor") }
                    TextField {
                        Layout.fillWidth: true
                        text: graphicsDevice.vendor
                        readOnly: true
                    }
                    Label { text: qsTr("GLSL") }
                    TextField {
                        Layout.fillWidth: true
                        text: graphicsDevice.shadingLanguage
                        readOnly: true
                    }
                    Label {
                        Layout.columnSpan: 2
                        text: qsTr("Available Extensions")
                    }
                    TextArea {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: graphicsDevice.extensionsText
                    }
                }
            }
            Tab {
                id: preferenceTab
                title: qsTr("Application")
                anchors.margins: tabView.anchors.margins
                ColumnLayout {
                    RowLayout {
                        id: fontFamilyBox
                        Label { text: qsTr("Font Family") }
                        ComboBox {
                            Layout.fillWidth: true
                            editable: true
                            model: Qt.fontFamilies()
                            currentIndex: Qt.fontFamilies().indexOf(applicationPreference.fontFamily)
                            onCurrentTextChanged: applicationPreference.fontFamily = currentText
                        }
                    }
                    GroupBox {
                        Layout.fillWidth: true
                        title: qsTr("Settings that is required restarting %1 to affect").arg(Qt.application.name)
                        ColumnLayout {
                            RowLayout {
                                Label { text: qsTr("Samples") }
                                SpinBox {
                                    minimumValue: 0
                                    maximumValue: 16
                                    value: applicationPreference.samples
                                    onValueChanged: applicationPreference.samples = value
                                }
                                Item { Layout.fillWidth: true }
                            }
                            RowLayout {
                                CheckBox {
                                    text: qsTr("Share Font Family to GUI")
                                    checked: applicationPreference.fontFamilyToGUIShared
                                    onCheckedChanged: applicationPreference.fontFamilyToGUIShared = checked
                                }
                                Item { Layout.fillWidth: true }
                            }
                            RowLayout {
                                CheckBox {
                                    text: qsTr("Enable Transparent Window")
                                    checked: applicationPreference.transparentWindowEnabled
                                    onCheckedChanged: applicationPreference.transparentWindowEnabled = checked
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
            Tab {
                anchors.margins: tabView.anchors.margins
                title: qsTr("Logging")
                ColumnLayout {
                    Layout.fillWidth: true
                    FileDialog {
                        id: loggingDirectoryDialog
                        folder: applicationPreference.baseLoggingDirectory
                        selectExisting: true
                        selectFolder: true
                        onAccepted: baseLoggingDirectory.text = fileUrl
                    }
                    Item { height: 5; Layout.fillWidth: true }
                    Label {
                        font.italic: true
                        font.pointSize: 16
                        text: qsTr("These settings are required restarting %1 to affect.").arg(Qt.application.name)
                    }
                    Item { height: 5; Layout.fillWidth: true }
                    GridLayout {
                        columns: 3
                        Label {
                            text: qsTr("Base Logging Directory")
                        }
                        Label {
                            text: qsTr("Suffix")
                        }
                        Label {
                            text: qsTr("Log Level")
                        }
                        TextField {
                            id: baseLoggingDirectory
                            Layout.fillWidth: true
                            text: applicationPreference.baseLoggingDirectory
                            onTextChanged: applicationPreference.baseLoggingDirectory = text
                        }
                        TextField {
                            text: applicationPreference.loggingDirectorySuffix
                            onTextChanged: applicationPreference.loggingDirectorySuffix = text
                        }
                        SpinBox {
                            minimumValue: 0
                            maximumValue: 3
                            value: applicationPreference.verboseLogLevel
                            onValueChanged: applicationPreference.verboseLogLevel = value
                        }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        Button {
                            text: qsTr("Change Location")
                            onClicked: loggingDirectoryDialog.open()
                        }
                        Button {
                            text: qsTr("Open Location")
                            onClicked: Qt.openUrlExternally(baseLoggingDirectory.text)
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
