import QtQuick
import QtQuick.Layouts
import SolTrace

RowLayout {
    id: root
    required property SplitPanelData target

    Layout.minimumWidth: implicitWidth
    spacing: 5

    STIconButton {
        id: close_button

        Layout.preferredWidth: implicitWidth
        Layout.preferredHeight: implicitWidth

        icon: "\uf00d"
        toolTip: "Close Panel"
        onClicked: root.target.visible = false
    }
}
