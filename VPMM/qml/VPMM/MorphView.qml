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

Item {
    id: morphView
    property var targetObject
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        GridLayout {
            columns: 2
            Label { text: qsTr("Name") }
            TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Input Morph Name Here")
                text: targetObject.name
            }
            Label { text: qsTr("Category") }
            ComboBox {
                id: morphCategoryComboBox
                function indexOf(value) {
                    var result = model.filter(function(element){ return element.value === value })
                    return result.length > 0 ? (result[0].value - 1) : -1
                }
                Layout.fillWidth: true
                model: [
                    { "text": qsTr("Lip"), "value": VPMM.Morph.Lip },
                    { "text": qsTr("Eye"), "value": VPMM.Morph.Eye },
                    { "text": qsTr("Eyeblow"), "value": VPMM.Morph.Eyeblow },
                    { "text": qsTr("Other"), "value": VPMM.Morph.Other }
                ]
                currentIndex: indexOf(targetObject.category)
            }
            Binding {
                target: targetObject
                property: "category"
                value: morphCategoryComboBox.model[morphCategoryComboBox.currentIndex].value
            }
            Label { text: qsTr("Type") }
            ComboBox {
                id: morphTypeComboBox
                function indexOf(value) {
                    var result = model.filter(function(element){ return element.value === value })
                    return result.length > 0 ? result[0].value : -1
                }
                Layout.fillWidth: true
                model: [
                    { "text": qsTr("Group"), "value": VPMM.Morph.Group },
                    { "text": qsTr("Vertex"), "value": VPMM.Morph.Vertex },
                    { "text": qsTr("Bone"), "value": VPMM.Morph.Bone },
                    { "text": qsTr("TexCoord"), "value": VPMM.Morph.TexCoord },
                    { "text": qsTr("UVA1"), "value": VPMM.Morph.UVA1 },
                    { "text": qsTr("UVA2"), "value": VPMM.Morph.UVA2 },
                    { "text": qsTr("UVA3"), "value": VPMM.Morph.UVA3 },
                    { "text": qsTr("UVA4"), "value": VPMM.Morph.UVA4 },
                    { "text": qsTr("Material"), "value": VPMM.Morph.Material },
                    { "text": qsTr("Flip"), "value": VPMM.Morph.Flip },
                    { "text": qsTr("Impulse"), "value": VPMM.Morph.Impulse }
                ]
                currentIndex: indexOf(targetObject.type)
            }
            Binding {
                target: targetObject
                property: "type"
                value: morphTypeComboBox.model[morphTypeComboBox.currentIndex].value
            }
        }
        TableView {
            id: objectsInLabelTableView
            function toggleVisible(value, item) {
                switch (targetObject.type) {
                case VPMM.Morph.Group:
                    break
                case VPMM.Morph.Vertex:
                    objectsInLabelTableView.visible = value
                    childVertexMorphView.visible = !objectsInLabelTableView.visible
                    break
                case VPMM.Morph.Bone:
                    break
                case VPMM.Morph.TexCord:
                case VPMM.Morph.UVA1:
                case VPMM.Morph.UVA2:
                case VPMM.Morph.UVA3:
                case VPMM.Morph.UVA4:
                    break
                case VPMM.Morph.Material:
                    break
                case VPMM.Morph.Flip:
                    break
                case VPMM.Morph.Impulse:
                    break
                }
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: {
                switch (targetObject.type) {
                case VPMM.Morph.Group:
                    return targetObject.groups
                case VPMM.Morph.Vertex:
                    return targetObject.vertices
                case VPMM.Morph.Bone:
                    return targetObject.bones
                case VPMM.Morph.TexCord:
                case VPMM.Morph.UVA1:
                case VPMM.Morph.UVA2:
                case VPMM.Morph.UVA3:
                case VPMM.Morph.UVA4:
                    return targetObject.uvs
                case VPMM.Morph.Material:
                    return targetObject.materials
                case VPMM.Morph.Flip:
                    return targetObject.flips
                case VPMM.Morph.Impulse:
                    return targetObject.impulses
                }
            }
            selectionMode: SelectionMode.ExtendedSelection
            TableViewColumn { title: qsTr("Index"); role: "index" }
            TableViewColumn { title: qsTr("Name"); role: "name" }
            onDoubleClicked: toggleVisible(false, model[row])
        }
        SystemPalette { id: systemPalette }
        Rectangle {
            id: childVertexMorphView
            property var item: objectsInLabelTableView.model[objectsInLabelTableView.currentRow]
            anchors.fill: objectsInLabelTableView
            anchors.margins: 10
            color: systemPalette.window
            visible: false
            VPMM.Vector3 { id: childVertexMorphPosition; value: item ? item.position : Qt.vector3d(0, 0, 0) }
            Binding {
                target: item
                property: "position"
                value: childVertexMorphPosition.value
                when: childVertexMorphPositionXSpinBox.hovered || childVertexMorphPositionYSpinBox.hovered || childVertexMorphPositionZSpinBox.hovered
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                GroupBox {
                    title: qsTr("Position")
                    GridLayout {
                        columns: 2
                        Label { text: "X" }
                        SpinBox {
                            id: childVertexMorphPositionXSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: childVertexMorphPosition.x
                        }
                        Label { text: "Y" }
                        SpinBox {
                            id: childVertexMorphPositionYSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: childVertexMorphPosition.y
                        }
                        Label { text: "Z" }
                        SpinBox {
                            id: childVertexMorphPositionZSpinBox
                            maximumValue: 100000
                            minimumValue: -maximumValue
                            decimals: 5
                            stepSize: 0.01
                            value: childVertexMorphPosition.z
                        }
                    }
                }
                Button {
                    text: qsTr("Back to vertex morph list")
                    onClicked: objectsInLabelTableView.toggleVisible(true)
                }
            }
        }
    }
}

