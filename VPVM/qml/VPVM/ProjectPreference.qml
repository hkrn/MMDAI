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
                    RowLayout {
                        Label { text: qsTr("Title") }
                        TextField {
                            Layout.fillWidth: true
                            text: scene.project.title
                            onAccepted: scene.project.title = text
                        }
                    }
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
                        ColumnLayout {
                            ColumnLayout {
                                RowLayout {
                                    ComboBox {
                                        Layout.fillWidth: true
                                        model: [
                                            qsTr("No Acceleration (CPU)"),
                                            qsTr("Parallel (CPU)"),
                                            qsTr("Vertex Shader (GPU)"),
                                            qsTr("OpenCL (GPU)"),
                                            qsTr("OpenCL (CPU)")
                                        ]
                                        onCurrentIndexChanged: {
                                            switch (currentIndex) {
                                            case 0:
                                            default:
                                                scene.project.accelerationType = VPVM.Project.NoAcceleration
                                                break;
                                            case 1:
                                                scene.project.accelerationType = VPVM.Project.ParallelAcceleration
                                                break;
                                            case 2:
                                                scene.project.accelerationType = VPVM.Project.VertexShaderAcceleration
                                                break;
                                            case 3:
                                                scene.project.accelerationType = VPVM.Project.OpenCLCPUAcceleration
                                                break;
                                            case 4:
                                                scene.project.accelerationType = VPVM.Project.OpenCLGPUAcceleration
                                                break;
                                            }
                                        }
                                    }
                                }
                                Label {
                                    text: qsTr("OpenCL acceleration affects after loading model, remains are not affected.")
                                }
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Misc")
                        Layout.fillWidth: true
                        ColorDialog {
                            id: colorDialog
                            title: qsTr("Color for background")
                            color: scene.project.screenColor
                            showAlphaChannel: applicationPreference.transparentWindowEnabled
                            onAccepted: scene.project.screenColor = color
                        }
                        ColumnLayout {
                            RowLayout {
                                Rectangle {
                                    height: 20
                                    width: 20
                                    color: scene.project.screenColor
                                }
                                Button {
                                    text: qsTr("Background Color")
                                    onClicked: colorDialog.open()
                                }
                                CheckBox {
                                    text: qsTr("Show Grid")
                                    checked: scene.project.gridVisible
                                    onCheckedChanged: scene.project.gridVisible = checked
                                }
                            }
                            RowLayout {
                                Label { text: qsTr("Shadow Map Size") }
                                ComboBox {
                                    property var __model: [
                                        { "text": qsTr("Normal (1024x1024)"),    "width": 1024 },
                                        { "text": qsTr("High (2048x2048)"),      "width": 2048 },
                                        { "text": qsTr("Very High (4096x4096)"), "width": 4096 }
                                    ]
                                    model: __model
                                    onCurrentIndexChanged: {
                                        var width = __model[currentIndex].width
                                        scene.shadowMapSize = Qt.vector3d(width, width, 1)
                                    }
                                }
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
            Tab {
                title: qsTr("Render Order")
                anchors.margins: tabView.anchors.margins
                ColumnLayout {
                    ListModel {
                        id: modelList
                        function __updateData() {
                            clear()
                            var models = scene.project.availableModels, allModels = [], i
                            for (i in models) {
                                allModels.push(models[i])
                            }
                            allModels.sort(function(a, b) { return a.orderIndex - b.orderIndex })
                            for (i in allModels) {
                                var model = allModels[i]
                                append({ "model": model })
                            }
                        }
                        function updateOrderIndices() {
                            var models = scene.project.availableModels
                            for (var i = 0; i < count; i++) {
                                var model = get(i).model
                                model.orderIndex = i + 1
                            }
                        }
                        Component.onCompleted: {
                            scene.project.availableModelsChanged.connect(__updateData)
                            __updateData()
                        }
                    }
                    TableView {
                        id: modelListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        headerVisible: false
                        model: modelList
                        TableViewColumn { role: "name"; title: "name" }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        Button {
                            text: qsTr("Up")
                            onClicked: {
                                var currentIndex = modelListView.currentRow
                                if (currentIndex > 0) {
                                    modelList.move(currentIndex, currentIndex - 1, 1)
                                    modelList.updateOrderIndices()
                                }
                            }
                        }
                        Button {
                            text: qsTr("Down")
                            onClicked: {
                                var currentIndex = modelListView.currentRow
                                if (currentIndex != -1 && currentIndex < modelList.count - 1) {
                                    modelList.move(currentIndex, currentIndex + 1, 1)
                                    modelList.updateOrderIndices()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
