import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STPopup {
    id: root

    required property var filterModel

    margins: 1
    padding: 8

    ColumnLayout {
        spacing: 6

        Label {
            Layout.fillWidth: true
            text: "Ray Contributions"
            font.bold: true
        }

        STSwitch {
            Layout.fillWidth: true
            text: "Include Direct Sun Rays"
            checked: root.filterModel.include_direct_sun_rays
            onToggled: root.filterModel.include_direct_sun_rays = checked

            ToolTip.visible: hovered
            ToolTip.text: "Include hits with no previous physical ray interaction."
        }

        STSwitch {
            Layout.fillWidth: true
            text: "Only Rays Terminating Here"
            checked: root.filterModel.terminal_interactions_only
            onToggled: root.filterModel.terminal_interactions_only = checked

            ToolTip.visible: hovered
            ToolTip.text: "Include only rays absorbed by the mapped element."
        }
    }
}
