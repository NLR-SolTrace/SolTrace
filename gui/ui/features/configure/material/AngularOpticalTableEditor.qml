import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SolTrace

STDialog {
    id: root

    required property var tableModel
    required property string quantityName
    required property bool tableEnabled

    signal tableEnabledRequested(bool enabled)

    title: qsTr("%1 by incidence angle").arg(quantityName)
    width: 520
    height: 440
    modal: true
    standardButtons: Dialog.Close

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        STSwitch {
            text: qsTr("Use angle-dependent %1").arg(root.quantityName.toLowerCase())
            checked: root.tableEnabled
            onToggled: root.tableEnabledRequested(checked)
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Values are linearly interpolated between incidence angles. Angles must be in strictly increasing order.")
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            opacity: 0.75
        }

        ListView {
            id: tableView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: root.tableModel

            ScrollBar.vertical: STScrollBar { }

            header: RowLayout {
                width: tableView.width
                spacing: 6

                Item {
                    Layout.preferredWidth: 28
                }

                Label {
                    Layout.preferredWidth: 190
                    text: qsTr("Incidence angle (mrad)")
                    font.bold: true
                }

                Label {
                    Layout.fillWidth: true
                    text: root.quantityName
                    font.bold: true
                }

                Item {
                    Layout.preferredWidth: 32
                }
            }

            delegate: RowLayout {
                id: tableRow

                required property int index

                width: tableView.width
                spacing: 6

                Label {
                    Layout.preferredWidth: 28
                    text: tableRow.index + 1
                    horizontalAlignment: Text.AlignHCenter
                }

                STDoubleSpinBox {
                    Layout.preferredWidth: 190
                    from: tableRow.index > 0
                          ? root.tableModel.data(root.tableModel.index(
                                                     tableRow.index - 1, 0)) + 0.001
                          : 0
                    to: tableRow.index + 1 < root.tableModel.count
                        ? root.tableModel.data(root.tableModel.index(
                                                   tableRow.index + 1, 0)) - 0.001
                        : 3141.592
                    decimals: 3
                    stepSize: 10
                    value: root.tableModel.data(
                               root.tableModel.index(tableRow.index, 0))
                    onValueModified: root.tableModel.setData(
                                         root.tableModel.index(tableRow.index, 0),
                                         value,
                                         Qt.EditRole)
                }

                STDoubleSpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    decimals: 4
                    stepSize: 0.01
                    value: root.tableModel.data(
                               root.tableModel.index(tableRow.index, 1))
                    onValueModified: root.tableModel.setData(
                                         root.tableModel.index(tableRow.index, 1),
                                         value,
                                         Qt.EditRole)
                }

                STIconButton {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    icon: "\uf00d"
                    iconSize: 10
                    enabled: root.tableModel.count > 1
                    toolTip: qsTr("Remove row")
                    onClicked: root.tableModel.remove(tableRow.index)
                }
            }

            footer: STButton {
                width: tableView.width
                text: qsTr("+ Add row")
                onClicked: root.tableModel.append()
            }
        }
    }
}
