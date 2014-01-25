import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

ScrollView {
    Item {
        id: softBodyView
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            RowLayout {
                Label { text: qsTr("Name") }
                TextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Input Soft Body Name Here")
                }
            }
            Item { Layout.fillHeight: true }
        }
    }
}
