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
            CheckBox { text: qsTr("Special") }
        }
        TableView {
            model: labelTypeComboBox.currentIndex === 0 ? targetObject.bones : targetObject.morphs
            Layout.fillWidth: true
            Layout.fillHeight: true
            TableViewColumn { title: qsTr("Name"); role: "name" }
        }
        RowLayout {
            ComboBox {
                Layout.fillWidth: true
                model: labelTypeComboBox.currentIndex === 0 ? bonesModel : morphsModel
                editable: true
            }
            Button {
                text: qsTr("Add")
            }
            Button {
                text: qsTr("Remove")
            }
        }
    }
}
