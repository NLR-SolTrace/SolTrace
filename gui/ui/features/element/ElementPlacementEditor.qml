import QtQuick
import QtQuick.Layouts

import SolTrace

STFormPanel {
    id: root

    property var module

    STPropertySeparator {
        title: "Placement"
    }

    STSwitch {
        text: "Use 3D widget"
        checked: App.view.mouse_mode === ViewModule.EditElement
        onToggled: App.view.mouse_mode = checked ? ViewModule.EditElement
                                                  : ViewModule.Camera
    }

    InlineDocumentation {
        key: "configure.layout.coordinates"
        Layout.fillWidth: true
    }

    STFormRow {
        label: "Coordinates"

        STComboBox {
            id: positionModeCombo
            Layout.fillWidth: true
            model: ["Local", "Global"]
        }
    }

    StackLayout {
        id: positionSwipe
        Layout.fillWidth: true
        currentIndex: positionModeCombo.currentIndex

        ElementPositionEditor {
            Layout.preferredWidth: positionSwipe.width
            module: root.module
            targetProperty: "position"
            resetText: "Reset Local Default Position"
        }

        ElementPositionEditor {
            Layout.preferredWidth: positionSwipe.width
            module: root.module
            targetProperty: "global_position"
            resetText: "Reset Global Default Position"
        }
    }

    ElementRotationEditor {
        Layout.fillWidth: true
        module: root.module
    }
}
