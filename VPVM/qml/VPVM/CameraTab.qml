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

Tab {
    id: cameraTab
    title: qsTr("Camera")
    anchors.margins: propertyPanel.anchors.margins
    RowLayout {
        GroupBox {
            title: qsTr("Camera")
            Layout.fillHeight: true
            RowLayout {
                AxesSpinBox {
                    title: qsTr("LookAt")
                    Layout.fillHeight: true
                    Layout.columnSpan: 2
                    minimumValue: propertyPanel.minimumPositionValue
                    maximumValue: propertyPanel.maximumPositionValue
                    decimals: propertyPanel.positionDecimalPrecision
                    stepSize: propertyPanel.positionStepSize
                    value: scene.camera.lookAt
                }
                AxesSpinBox {
                    title: qsTr("Angle")
                    Layout.fillHeight: true
                    Layout.columnSpan: 2
                    minimumValue: propertyPanel.minimumRotaitonValue
                    maximumValue: propertyPanel.maximumRotaitonValue
                    decimals: propertyPanel.rotationDecimalPrecision
                    stepSize: propertyPanel.rotationStepSize
                    value: scene.camera.angle
                }
                ColumnLayout {
                    GroupBox {
                        title: qsTr("Look")
                        GridLayout {
                            columns: 2
                            Label { text: qsTr("Fov") }
                            SpinBox {
                                maximumValue: 135
                                minimumValue: -135
                                value: scene.camera.fov
                                onValueChanged: scene.camera.fov = value
                            }
                            Label { text: qsTr("Distance") }
                            SpinBox {
                                maximumValue: propertyPanel.maximumPositionValue
                                minimumValue: propertyPanel.minimumPositionValue
                                decimals: propertyPanel.positionDecimalPrecision
                                stepSize: propertyPanel.positionStepSize
                                value: scene.camera.distance
                                onValueChanged: if (hovered) scene.camera.distance = value
                            }
                        }
                    }
                    Button {
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Register")
                        onClicked: {
                            var camera = scene.camera
                            camera.motion.addKeyframe(camera, timeline.timeIndex)
                        }
                    }
                }
            }
        }
        InterpolationPanel {
            targetKeyframe : (enabled && scene.currentMotion) ? scene.currentMotion.resolveKeyframeAt(timeline.timeIndex, scene.camera) : null
            enabled: scene.currentMotion
            type: 0
            typeModel: [
                qsTr("X Axis"),
                qsTr("Y Axis"),
                qsTr("Z Axis"),
                qsTr("Angle"),
                qsTr("Distance"),
                qsTr("Fov")
            ]
        }
        Item { Layout.fillWidth: true }
    }
}
