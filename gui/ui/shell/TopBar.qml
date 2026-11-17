import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import SolTrace

Item {
    id: root

    required property int available_width
    required property var blur_source

    signal show_script_area()

    TopBarLeftPane {
        id: top_bar_left_pane

        visible: !App.view.left_panel.visible
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: implicitWidth

        available_width: root.available_width
        show_logos: false
        show_branding: false
        show_context: false
        show_left_panel_toggle: true
    }

    Item {
        id: workflow_slot

        anchors.left: parent.left
        anchors.leftMargin: top_bar_left_pane.visible
                            ? top_bar_left_pane.width + 20 : 0
        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(workflow_bar.implicitWidth,
                        Math.max(0,
                                 right_controls.x - workflow_slot.x - 10))
        height: Math.min(42, root.height)

        anchors.topMargin: 5

        WorkflowPane {
            id: workflow_bar
            anchors.fill: parent

            blur_source: root.blur_source
            show_header: false
        }
    }

    ShadowedGlassRectangle {
        id: right_controls

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: top_bar_right_pane.implicitWidth + 16
        height: Math.min(40, root.height)
        radius: height / 2

        blur_source: root.blur_source
        glassColor: App.theme.glassColor

        TopBarRightPane {
            id: top_bar_right_pane

            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            blur_source: root.blur_source
            available_width: root.available_width
        }
    }
}
