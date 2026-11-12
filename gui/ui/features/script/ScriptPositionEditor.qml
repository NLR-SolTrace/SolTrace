import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STPopup {
    id: root

    required property var module
    property int previousMouseMode: ViewModule.Camera

    parent: Overlay.overlay
    modal: false
    focus: true
    enable_shadow: true
    closePolicy: Popup.CloseOnEscape

    width: 320
    x: Math.max(12, parent.width - width - 24)
    y: Math.max(12, Math.min(90, parent.height - height - 12))

    onOpened: {
        previousMouseMode = App.view.mouse_mode
        App.view.mouse_mode = ViewModule.EditPosition
        refresh()
    }

    onClosed: {
        root.module.end()
        if (App.view.mouse_mode === ViewModule.EditPosition)
            App.view.mouse_mode = previousMouseMode
    }

    function refresh() {
        const position = root.module.position
        x_position.value = position.x
        y_position.value = position.y
        z_position.value = position.z
    }

    function updatePosition() {
        root.module.position = Qt.vector3d(x_position.value,
                                           y_position.value,
                                           z_position.value)
    }

    Connections {
        target: root.module

        function onPosition_changed() {
            root.refresh()
        }
    }

    contentItem: ColumnLayout {
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    Layout.fillWidth: true
                    text: root.module.name
                    font.bold: true
                    elide: Label.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Global position")
                    opacity: 0.65
                }
            }

            STIconButton {
                icon: "\uf00d"
                toolTip: qsTr("Close Position Editor")
                onClicked: root.close()
            }
        }

        STFormRow {
            label: qsTr("X")

            STDoubleSpinBox {
                id: x_position
                Layout.fillWidth: true
                from: -Infinity
                to: Infinity
                stepSize: root.module.step
                decimals: root.module.decimals
                onValueModified: root.updatePosition()
            }
        }

        STFormRow {
            label: qsTr("Y")

            STDoubleSpinBox {
                id: y_position
                Layout.fillWidth: true
                from: -Infinity
                to: Infinity
                stepSize: root.module.step
                decimals: root.module.decimals
                onValueModified: root.updatePosition()
            }
        }

        STFormRow {
            label: qsTr("Z")

            STDoubleSpinBox {
                id: z_position
                Layout.fillWidth: true
                from: -Infinity
                to: Infinity
                stepSize: root.module.step
                decimals: root.module.decimals
                onValueModified: root.updatePosition()
            }
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Drag the viewport widget to adjust this point.")
            opacity: 0.65
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }

        STButton {
            Layout.fillWidth: true
            text: qsTr("Done")
            onClicked: root.close()
        }
    }
}
