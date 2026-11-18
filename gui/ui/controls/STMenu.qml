import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Menu {
    id: control

    property real minimumWidth: 150

    readonly property real widestItemWidth: {
        let width = 0

        for (let index = 0; index < count; ++index) {
            const item = itemAt(index)
            if (!item) {
                continue
            }

            const contentWidth = item.implicitContentWidth || 0
            const itemPadding = (item.leftPadding || 0)
                                + (item.rightPadding || 0)
            width = Math.max(width, contentWidth + itemPadding)
        }

        return width
    }

    implicitWidth: Math.max(minimumWidth,
                            widestItemWidth + leftPadding + rightPadding)

    background: Rectangle {
        implicitWidth: control.minimumWidth
        implicitHeight: 40

        radius: 14

        color: Qt.alpha(Material.backgroundColor, .90)
    }
}
