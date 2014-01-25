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
                text: labelView.targetObject.name
            }
        }
        Item { Layout.fillHeight: true }
    }
}
