import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SolTrace

STIconButton {
    id: root

    property string currentName: ""
    property string title: qsTr("Rename Item")

    signal renameRequested(string name)

    icon: "\uf303"
    toolTip: root.title
    onClicked: rename_dialog.open()

    STDialog {
        id: rename_dialog
        title: root.title
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAboutToShow: {
            name_field.text = root.currentName
            name_field.selectAll()
            name_field.forceActiveFocus()
        }

        onAccepted: root.renameRequested(name_field.text)

        RowLayout {
            anchors.fill: parent

            Label {
                text: qsTr("Name:")
            }

            STTextField {
                id: name_field
                Layout.fillWidth: true
                onAccepted: rename_dialog.accept()
            }
        }
    }
}
