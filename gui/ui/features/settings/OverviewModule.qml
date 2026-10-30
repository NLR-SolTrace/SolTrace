import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import SolTrace

STScrollView {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentWidth: availableWidth

    ColumnLayout {
        width: root.availableWidth
        spacing: 12

        InlineDocumentation {
            key: "settings.overview"
            expanded: true
            Layout.fillWidth: true
        }
        InlineDocumentation {
            key: "settings.background"
            expanded: true
            Layout.fillWidth: true
        }
        InlineDocumentation {
            key: "settings.methodology"
            expanded: true
            Layout.fillWidth: true
        }
    }
}
