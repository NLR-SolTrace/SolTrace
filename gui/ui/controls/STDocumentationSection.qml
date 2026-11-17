import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root

    required property string key
    property bool expanded: false

    readonly property int docsVersion: App.docs.version

    Layout.fillWidth: true

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Label {
            text: {
                App.docs.locale
                App.docs.version
                return App.docs.get(root.key, "title")
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Material.dividerColor
        }

        STClickableLabel {
            text: root.expanded ? qsTr("Hide help") : qsTr("Help")
            color: Material.accentColor
            font.underline: containsMouse
            onClicked: root.expanded = !root.expanded
        }
    }

    DocumentationContent {
        key: root.key
        visible: root.expanded
        Layout.fillWidth: true
    }
}
