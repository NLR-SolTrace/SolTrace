import QtQuick.Controls
import QtQuick.Layouts

import SolTrace

STFormPanel {
    id: root

    property var module
    title: qsTr("Visibility & Simulation")

    ColorPickerField {
        id: elementColorPicker
        Layout.fillWidth: true
        color: root.module.color
        label: qsTr("Element Tint")
        onUpdated: {
            root.module.color = elementColorPicker.color
        }
    }

    STSwitch {
        text: qsTr("Show in 3D View")
        checked: !root.module.hidden
        onToggled: root.module.hidden = !checked
    }

    STSwitch {
        text: qsTr("Exclude from Simulation")
        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Keep this element visible in the 3D view, but omit it from ray tracing.")
        checked: root.module.disabled
        onToggled: root.module.disabled = checked
    }

    STSwitch {
        text: qsTr("Ray Probe")
        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Record rays at this element without changing their direction or power.")
        checked: root.module.virtual_element
        onToggled: root.module.virtual_element = checked
    }
}
