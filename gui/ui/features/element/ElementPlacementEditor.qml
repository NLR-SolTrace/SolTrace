import QtQuick
import QtQuick.Layouts

import SolTrace

STFormPanel {
    id: root

    property var module
    title: qsTr("Placement")

    STSwitch {
        text: qsTr("Use 3D Widget")
        checked: App.view.mouse_mode === ViewModule.EditElement
        onToggled: App.view.mouse_mode = checked ? ViewModule.EditElement
                                                  : ViewModule.Camera
    }

    STDocumentationSection {
        key: "configure.layout.coordinates"
    }

    STFormRow {
        label: qsTr("Coordinates")

        STTabBar {
            id: positionModeBar
            secondary: true
            Layout.fillWidth: true
            model: [qsTr("Local"), qsTr("Global")]
        }
    }

    StackLayout {
        id: positionSwipe
        Layout.fillWidth: true
        currentIndex: positionModeBar.currentIndex

        ElementPositionEditor {
            Layout.preferredWidth: positionSwipe.width
            module: root.module
            targetProperty: "position"
            resetText: qsTr("Reset Local Position")
        }

        ElementPositionEditor {
            Layout.preferredWidth: positionSwipe.width
            module: root.module
            targetProperty: "global_position"
            resetText: qsTr("Reset Global Position")
        }
    }

    ElementRotationEditor {
        Layout.fillWidth: true
        module: root.module
    }
}
