import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

Item {
    id: control

    implicitHeight: center_layout.implicitHeight + 12
    implicitWidth: 100

    Behavior on implicitHeight {
        NumberAnimation {
            duration: 100
        }
    }

    width: ListView.view ? ListView.view.width : implicitWidth

    default property alias content: children_area.data

    property bool highlighted: ListView.isCurrent ? ListView.isCurrent : false
    property bool hovered: mouse_area.containsMouse
    property bool down: mouse_area.containsPress

    property string text

    property bool expanded: highlighted

    property bool can_rename: true

    signal clicked
    signal deleteRequested
    signal rename(string text)

    clip: visible

    Rectangle {
        anchors.fill: parent

        opacity: control.enabled ? 1 : 0.3
        color: control.down
               ?
                   Material.rippleColor
                 :
                   control.highlighted
                   ?
                       Material.highlightedRippleColor
                     :
                       control.hovered
                       ?
                           Qt.alpha(Material.highlightedRippleColor, 0.35)
                         :
                           "transparent"
        radius: 14
    }

    MouseArea {
        id: mouse_area
        anchors.fill: parent

        hoverEnabled: true

        onClicked: control.clicked()
    }

    ColumnLayout {
        id: center_layout

        anchors.fill: parent
        anchors.margins: 6

        RowLayout {
            id: title_row
            Layout.fillWidth: true

            property bool is_editing: false

            Label {
                visible: !title_row.is_editing
                Layout.fillWidth: true
                text: control.text
                font.pointSize: 18
            }

            STTextField {
                visible: title_row.is_editing

                Layout.fillWidth: true
                text: control.text

                onTextEdited: {
                    if (title_row.is_editing) {
                        control.rename(text)
                    }
                }

                onAccepted: {
                    control.rename(text)
                    title_row.is_editing = false
                    focus = false
                }
            }

            STIconButton {
                visible: control.expanded && control.can_rename
                icon: "\uf303"

                onClicked: title_row.is_editing = !title_row.is_editing
            }
        }

        RowLayout {
            visible: control.expanded
            Layout.fillWidth: true
            id: children_area
        }
    }
}
