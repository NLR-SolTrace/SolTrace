import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SolTrace

GridLayout {
    id: root

    property string label: ""
    property int labelTextFormat: Text.PlainText

    default property alias content: contentRow.data

    Layout.fillWidth: true
    columns: 1
    columnSpacing: 8
    rowSpacing: 5

    STPropertyLabel {
        text: root.label
        textFormat: root.labelTextFormat
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
    }

    RowLayout {
        id: contentRow

        Layout.fillWidth: true
        spacing: 8
    }
}
