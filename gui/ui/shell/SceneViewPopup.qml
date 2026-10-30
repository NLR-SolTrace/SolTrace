import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STPopup {
    id: root

    property string title
    property string icon
    default property alias content: content_layout.data

    width: 280
    x: -width - 16
    y: (parent.height - height) / 2
    enable_shadow: true

    contentItem: ColumnLayout {
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                visible: root.icon.length > 0
                text: root.icon
                font.family: "Font Awesome 7 Free"
                color: App.theme.fontColor
            }

            SubHeader {
                Layout.fillWidth: true
                text: root.title
                elide: Label.ElideRight
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Material.dividerColor
        }

        ColumnLayout {
            id: content_layout
            Layout.fillWidth: true
            spacing: 8
        }
    }
}
