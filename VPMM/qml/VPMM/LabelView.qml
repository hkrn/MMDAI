import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

Item {
    id: labelView
    property var targetObject
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        RowLayout {
            Label { text: qsTr("Name") }
            TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Input Label Name Here")
                text: targetObject.name
            }
        }
        RowLayout {
            Label { text: qsTr("Type") }
            ComboBox {
                id: labelTypeComboBox
                model: [
                    qsTr("Bones"),
                    qsTr("Morphs")
                ]
            }
            CheckBox {
                id: labelSpecialCheckBox
                text: qsTr("Special")
                checked: targetObject.special
            }
            Binding {
                target: targetObject
                property: "special"
                value: labelSpecialCheckBox.checked
            }
        }
        TableView {
            id: objectsInLabelTableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: labelTypeComboBox.currentIndex === 0 ? targetObject.bones : targetObject.morphs
            selectionMode: SelectionMode.ExtendedSelection
            TableViewColumn { title: qsTr("Name"); role: "name" }
        }
        RowLayout {
            ComboBox {
                id: objectToAddComboBox
                Layout.fillWidth: true
                model: labelTypeComboBox.currentIndex === 0 ? bonesModel : morphsModel
                editable: true
            }
            Button {
                text: qsTr("Add")
                onClicked: targetObject.addObject(objectToAddComboBox.model.get(objectToAddComboBox.currentIndex).item)
            }
            Button {
                text: qsTr("Remove")
                enabled: objectsInLabelTableView.selection.count > 0
                onClicked: {
                    var removeItems = []
                    objectsInLabelTableView.selection.forEach(function(rowIndex) {
                        removeItems.push(objectsInLabelTableView.model[rowIndex])
                    })
                    removeItems.reverse()
                    removeItems.forEach(function(item){ targetObject.removeObject(item) })
                }
            }
        }
    }
}
