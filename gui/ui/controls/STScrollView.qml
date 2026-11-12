import QtQuick
import QtQuick.Controls

ScrollView {
    id: control

    ScrollBar.vertical: STScrollBar {
        parent: control
        x: control.mirrored ? 0 : control.width - width
        y: control.topPadding
        height: control.availableHeight
    }

    ScrollBar.horizontal: STScrollBar {
        parent: control
        x: control.leftPadding
        y: control.height - height
        width: control.availableWidth
    }
}
