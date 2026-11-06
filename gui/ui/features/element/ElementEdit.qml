import QtQuick
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root

    property var module: App.layout.instance_edit

    ElementAssignmentEditor {
        Layout.fillWidth: true
        module: root.module
    }

    ElementPlacementEditor {
        Layout.fillWidth: true
        module: root.module
    }

    ElementVisualizationEditor {
        Layout.fillWidth: true
        module: root.module
    }
}
