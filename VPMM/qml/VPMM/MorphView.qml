import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

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
                Layout.fillWidth: true
                model: [
                    qsTr("Lip"),
                    qsTr("Eye"),
                    qsTr("Eyeblow"),
                    qsTr("Others")
                ]
            }
            Label { text: qsTr("Type") }
            Label { text: qsTr("Type Name") }
        }
        Item { Layout.fillHeight: true }
    }
}

