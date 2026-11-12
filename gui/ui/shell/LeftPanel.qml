import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ShadowedGlassRectangle {
    id: root

    property int available_width: 0
    readonly property int header_height: 46

    radius: 10

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
        preventStealing: true

        onPressed: (mouse) => mouse.accepted = true
        onReleased: (mouse) => mouse.accepted = true
        onClicked: (mouse) => mouse.accepted = true
        onDoubleClicked: (mouse) => mouse.accepted = true
        onPositionChanged: (mouse) => mouse.accepted = true
        onWheel: (wheel) => wheel.accepted = true
    }

    RowLayout {
        id: module_info_row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        height: root.header_height

        TopBarLeftPane {
            Layout.fillHeight: true
            Layout.fillWidth: true

            available_width: root.available_width
            show_logos: true
            show_context: true
        }
    }

    Rectangle {
        anchors.left: module_info_row.left
        anchors.right: module_info_row.right
        height: 1
        anchors.bottom: module_info_row.bottom
        color: Material.dividerColor
    }

    StackLayout {
        id: module_stack
        currentIndex: Math.max(0, App.view.workflow_phase - 1)
        onCurrentIndexChanged: App.view.workflow_phase = currentIndex + 1

        anchors.top: module_info_row.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        anchors.leftMargin: 16
        anchors.rightMargin: 16

        LoadModule {}
        ConfigureModule {}
        SimulateModule {}
        AnalyzeModule {}
    }
}
