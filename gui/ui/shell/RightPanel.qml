import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

ShadowedGlassRectangle {
    id: root
    required property int available_width

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 5

            Label {
                id: info_row
                Layout.fillWidth: true

                text: "Scripting"
                font.pointSize: 16
                font.family: "CMU Serif"
                font.bold: true

                elide: Label.ElideRight
                verticalAlignment: Label.AlignVCenter
            }

            PanelButtons {
                id: panel_buttons
                target: App.view.right_panel
            }
        }

        ScriptInterface {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
