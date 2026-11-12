import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root
    required property string key

    property string title: ""
    property string body: ""
    property bool expanded: false
    readonly property int docsVersion: App.docs.version

    RowLayout {
        Layout.fillWidth: true

        SubHeader {
            id: title_label
            text: {
                App.docs.locale
                App.docs.version
                return root.title ? root.title : App.docs.get(root.key, "title")
            }

            Layout.fillWidth: true
            elide: Label.ElideRight
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
        body: root.body
        visible: root.expanded
        Layout.fillWidth: true
    }
}
