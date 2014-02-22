/**

 Copyright (c) 2010-2014  hkrn

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
import com.github.mmdai.VPMM 1.0 as VPMM

ScrollView {
    id: ikView
    property var targetObject
    Item {
        id: ikContentView
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            Component.onCompleted: ikContentView.height = childrenRect.height
            GroupBox {
                title: qsTr("Constraint")
                GridLayout {
                    columns: 2
                    Label { text: qsTr("Target Bone") }
                    ComboBox {
                        model: bonesModel
                        editable: true
                        Layout.fillWidth: true
                        currentIndex: bonesModel.indexOf(targetObject.effectorBone)
                    }
                    Label { text: qsTr("Number of Iterations") }
                    SpinBox {
                        maximumValue: 255
                        minimumValue: 0
                        value: targetObject.numIterations
                    }
                    Label { text: qsTr("Angle") }
                    SpinBox {
                        maximumValue: 360
                        minimumValue: -maximumValue
                        value: targetObject.angleLimit
                    }
                }
            }
            GroupBox {
                title: qsTr("Joint Bones")
                ColumnLayout {
                    TableView {
                        id: ikJointsModelView
                        property var currentJoint: targetObject.allChildJoints[currentRow]
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        model: targetObject.allChildJoints
                        TableViewColumn { title: "name"; role: "name" }
                    }
                    RowLayout {
                        Layout.columnSpan: 2
                        Label { text: qsTr("Link Bone") }
                        ComboBox {
                            model: bonesModel
                            editable: true
                        }
                        Button { text: qsTr("Add") }
                    }
                    CheckBox {
                        id: enableAngularLimitCheckbox
                        text: qsTr("Enable Angular Limit")
                        checked: {
                            if (ikJointsModelView.currentRow >= 0) {
                                var currentJoint = ikJointsModelView.currentJoint
                                return currentJoint ? currentJoint.hasAngleLimit : false
                            }
                            return false
                        }
                    }
                    RowLayout {
                        GroupBox {
                            title: qsTr("Lower Angle Limit")
                            enabled: enableAngularLimitCheckbox.checked
                            VPMM.Vector3 { id: ikJointUpperLimit; value: enableAngularLimitCheckbox.checked ? ikJointsModelView.currentJoint.degreeUpperLimit : Qt.vector3d(0, 0, 0) }
                            VPMM.Vector3 { id: ikJointLowerLimit; value: enableAngularLimitCheckbox.checked ? ikJointsModelView.currentJoint.degreeLowerLimit : Qt.vector3d(0, 0, 0) }
                            Binding {
                                target: targetObject
                                property: "upperLimit"
                                value: ikJointUpperLimit.value
                                when: ikLowerLimitXSpinBox.hovered || ikLowerLimitYSpinBox.hovered || ikLowerLimitZSpinBox.hovered
                            }
                            Binding {
                                target: targetObject
                                property: "lowerLimit"
                                value: ikJointLowerLimit.value
                                when: ikUpperLimitXSpinBox.hovered || ikUpperLimitYSpinBox.hovered || ikUpperLimitZSpinBox.hovered
                            }
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: ikLowerLimitXSpinBox
                                    maximumValue: 360
                                    minimumValue: -maximumValue
                                    decimals: 1
                                    stepSize: 1
                                    value: ikJointLowerLimit.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: ikLowerLimitYSpinBox
                                    maximumValue: 360
                                    minimumValue: -maximumValue
                                    decimals: 1
                                    stepSize: 1
                                    value: ikJointLowerLimit.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: ikLowerLimitZSpinBox
                                    maximumValue: 360
                                    minimumValue: -maximumValue
                                    decimals: 1
                                    stepSize: 1
                                    value: ikJointLowerLimit.z
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("Upper Angle Limit")
                            enabled: enableAngularLimitCheckbox.checked
                            GridLayout {
                                columns: 2
                                Label { text: "X" }
                                SpinBox {
                                    id: ikUpperLimitXSpinBox
                                    maximumValue: 360
                                    minimumValue: -maximumValue
                                    decimals: 1
                                    stepSize: 1
                                    value: ikJointUpperLimit.x
                                }
                                Label { text: "Y" }
                                SpinBox {
                                    id: ikUpperLimitYSpinBox
                                    maximumValue: 360
                                    minimumValue: -maximumValue
                                    decimals: 1
                                    stepSize: 1
                                    value: ikJointUpperLimit.y
                                }
                                Label { text: "Z" }
                                SpinBox {
                                    id: ikUpperLimitZSpinBox
                                    maximumValue: 360
                                    minimumValue: -maximumValue
                                    decimals: 1
                                    stepSize: 1
                                    value: ikJointUpperLimit.z
                                }
                            }
                        }
                    }
                }
            }
            Item { height: 20 }
        }
    }
}
