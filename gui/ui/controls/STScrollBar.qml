import QtQuick
import QtQuick.Controls

ScrollBar {
    id: control

    readonly property int restingThickness: 4
    readonly property int hoveredThickness: 12
    property bool pointerDrag: false
    readonly property bool expanded: hovered || pointerDrag

    // ScrollIndicator is display-only. Desktop scroll surfaces use this
    // control so the thumb can be clicked and dragged and the track clicked.
    policy: ScrollBar.AsNeeded
    interactive: true
    hoverEnabled: true
    minimumSize: 0.08

    // Stay out of the content's way until the pointer deliberately reaches
    // the scrollbar. Expanding the attached edge grows it into the view.
    implicitWidth: orientation === Qt.Vertical
                   ? (expanded ? hoveredThickness : restingThickness)
                   : 100
    implicitHeight: orientation === Qt.Horizontal
                    ? (expanded ? hoveredThickness : restingThickness)
                    : 100

    // Preserve the larger target during a mouse drag. A touch press does not
    // hover first, so touch and kinetic scrolling leave the bar thin.
    onPressedChanged: {
        if (pressed)
            pointerDrag = hovered
        else
            pointerDrag = false
    }

    Behavior on implicitWidth {
        NumberAnimation { duration: 100; easing.type: Easing.OutCubic }
    }

    Behavior on implicitHeight {
        NumberAnimation { duration: 100; easing.type: Easing.OutCubic }
    }
}
