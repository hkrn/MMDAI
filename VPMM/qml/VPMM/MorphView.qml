import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

Item {
    id: morphView
    property var targetObject
    Component.onCompleted: morphView.height = childrenRect.height
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
        }
    }
}

