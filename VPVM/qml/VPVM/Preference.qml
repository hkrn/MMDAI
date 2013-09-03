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
    id: projectPreferenceDialog
    property var scene
    title: qsTr("Project Preference")
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
                title: qsTr("Preference")
                anchors.margins: preferenceLayout.anchors.margins
                ColumnLayout {
                    GroupBox {
                        title: qsTr("Physics Simulation Settings")
                        Layout.fillWidth: true
                        ColumnLayout {
                            GroupBox {
                                title: qsTr("Activation State")
                                RowLayout {
                                    ExclusiveGroup {
                                        id: enablePhysicsExclusiveGroup
                                        onCurrentChanged: {
                                            var type;
                                            switch (current) {
                                            case enablePhysicsAnyTimeButton:
                                                type = VPVM.World.EnableSimulationAnytime
                                                break;
                                            case enablePhysicsPlayOnlyButton:
                                                type = VPVM.World.EnableSimulationPlayOnly
                                                break;
                                            case disablePhysicsButton:
                                            default:
                                                type = VPVM.World.DisableSimulation
                                                break;
                                            }
                                            scene.project.world.simulationType = type
                                        }
                                    }
                                    RadioButton {
                                        id: enablePhysicsAnyTimeButton
                                        text: qsTr("Enable (Anytime)")
                                        exclusiveGroup: enablePhysicsExclusiveGroup
                                    }
                                    RadioButton {
                                        id: enablePhysicsPlayOnlyButton
                                        text: qsTr("Enable (Play only)")
                                        exclusiveGroup: enablePhysicsExclusiveGroup
                                    }
                                    RadioButton {
                                        id: disablePhysicsButton
                                        text: qsTr("Disable")
                                        checked: true
                                        exclusiveGroup: enablePhysicsExclusiveGroup
                                    }
                                }
                            }
                            RowLayout {
                                enabled: enablePhysicsExclusiveGroup.current !== disablePhysicsButton
                                AxesSpinBox {
                                    id: gravitySpinBox
                                    title: qsTr("Gravity and Direction")
                                    Layout.columnSpan: 2
                                    maximumValue: Math.pow(2, 31)
                                    decimals: 3
                                    value: scene.project.world.gravity
                                }
                                GroupBox {
                                    title: qsTr("Parameter")
                                    Layout.fillHeight: true
                                    GridLayout {
                                        columns: 2
                                        Label { text: qsTr("Seed") }
                                        SpinBox {
                                            minimumValue: 0
                                            maximumValue: Math.pow(2, 31)
                                            value: scene.project.world.randSeed
                                            onValueChanged: scene.project.world.randSeed = value
                                        }
                                        Label { text: qsTr("SubStep") }
                                        SpinBox { value: 2 }
                                        CheckBox {
                                            Layout.columnSpan: 2
                                            text: qsTr("Enable Floor")
                                            checked: scene.project.world.enableFloor
                                            onCheckedChanged: scene.project.world.enableFloor = checked
                                        }
                                    }
                                }
                                Item { Layout.columnSpan: 2 }
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Acceleration Type")
                        Layout.fillWidth: true
                        RowLayout {
                            ExclusiveGroup { id: accelerationTypeExclusiveGroup }
                            RadioButton {
                                text: qsTr("Software")
                                checked: true
                                exclusiveGroup: accelerationTypeExclusiveGroup
                            }
                            RadioButton {
                                text: qsTr("Parallel")
                                exclusiveGroup: accelerationTypeExclusiveGroup
                            }
                            RadioButton {
                                text: qsTr("OpenCL (GPU)")
                                exclusiveGroup: accelerationTypeExclusiveGroup
                            }
                            RadioButton {
                                text: qsTr("OpenCL (CPU)")
                                exclusiveGroup: accelerationTypeExclusiveGroup
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Misc")
                        Layout.fillWidth: true
                        RowLayout {
                            CheckBox { text: qsTr("Show Grid"); checked: true }
                        }
                    }
                    Item { Layout.fillHeight: true }
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
        }
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            anchors.margins: preferenceLayout.anchors.margins
            Button {
                text: qsTr("Apply")
                isDefault: true
                onClicked: {}
            }
            Button {
                text: qsTr("Cancel")
                onClicked: projectPreferenceDialog.close()
            }
            Button {
                text: qsTr("OK")
                onClicked: projectPreferenceDialog.close()
            }
        }
    }
}
