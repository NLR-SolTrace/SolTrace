import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import SolTrace

STFormPanel {
    id: root
    property bool has_selection : false
    property var material_editor : App.materials.group_edit
    property var side_editor
    property bool reflection: true

    STFormRow {
        label: qsTr("Error profile")

        STComboBox {
            id: distributionCombo
            Layout.fillWidth: true
            model: App.materials.material_edit.distribution_type_model
            textRole: "name"
            valueRole: "value"

            ToolTip.delay: 500
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Controls how slope and specularity errors perturb traced rays.")

            currentValue: root.side_editor.error_distribution_type

            onActivated: {
                if (root.side_editor) {
                    root.side_editor.error_distribution_type = currentValue
                }
            }
        }
    }


    // =========================================================================

    STFormRow {
        visible: root.reflection
        label: qsTr("Reflectance (ρ)")

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0.0
            to: 1.0
            decimals: 4
            stepSize: 0.01
            enabled: root.side_editor
                     && !root.side_editor.reflectivity_uses_table
            value: root.side_editor ? root.side_editor.reflectivity : 0
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.reflectivity = value
                }
            }
        }

        STIconButton {
            icon: "\uf201"
            label: root.side_editor
                   && root.side_editor.reflectivity_uses_table
                   ? qsTr("Edit table") : qsTr("Angle table")
            toolTip: qsTr("Configure reflectance by incidence angle")
            onClicked: reflectanceTableEditor.open()

            AngularOpticalTableEditor {
                id: reflectanceTableEditor
                quantityName: qsTr("Reflectance")
                graphColor: Material.color(Material.Blue)
                tableModel: root.side_editor
                            ? root.side_editor.reflectivity_table : null
                tableEnabled: root.side_editor
                              ? root.side_editor.reflectivity_uses_table : false
                onTableEnabledRequested: function(enabled) {
                    if (root.side_editor) {
                        root.side_editor.reflectivity_uses_table = enabled
                    }
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        visible: !root.reflection
        label: qsTr("Transmittance (τ)")

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0
            to: 1.0
            decimals: 4
            stepSize: 0.01
            enabled: root.side_editor
                     && !root.side_editor.transmissivity_uses_table
            value: root.side_editor ? root.side_editor.transmissivity : 0
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.transmissivity = value
                }
            }
        }

        STIconButton {
            icon: "\uf201"
            label: root.side_editor
                   && root.side_editor.transmissivity_uses_table
                   ? qsTr("Edit table") : qsTr("Angle table")
            toolTip: qsTr("Configure transmittance by incidence angle")
            onClicked: transmittanceTableEditor.open()

            AngularOpticalTableEditor {
                id: transmittanceTableEditor
                quantityName: qsTr("Transmittance")
                graphColor: Material.color(Material.Green)
                tableModel: root.side_editor
                            ? root.side_editor.transmissivity_table : null
                tableEnabled: root.side_editor
                              ? root.side_editor.transmissivity_uses_table : false
                onTableEnabledRequested: function(enabled) {
                    if (root.side_editor) {
                        root.side_editor.transmissivity_uses_table = enabled
                    }
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("Slope error (mrad)")

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0
            to: 1000
            decimals: 3
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.slope_error : 0

            ToolTip.delay: 500
            ToolTip.visible: hovered
            ToolTip.text: qsTr("σslope — angular variation of the surface normal.")

            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.slope_error = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("Specularity error (mrad)")

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0
            to: 1000
            decimals: 3
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.specularity_error : 0

            ToolTip.delay: 500
            ToolTip.visible: hovered
            ToolTip.text: qsTr("σspec — angular spread around the ideal reflected or transmitted ray.")

            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.specularity_error = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("Quick setup")

        STIconButton {
            icon: "\uf0d0"
            label: qsTr("Choose preset")
            toolTip: qsTr("Apply a preset to this face")
            onClicked: face_preset_menu.open()

            STMenu {
                id: face_preset_menu

                MenuItem {
                    text: qsTr("Ideal absorber")
                    onTriggered: {
                        if (root.side_editor) {
                            root.side_editor.set_ideal_absorption()
                        }
                    }
                }

                MenuItem {
                    text: qsTr("Ideal reflector")
                    onTriggered: {
                        if (root.side_editor) {
                            root.side_editor.set_ideal_reflection()
                        }
                    }
                }
            }
        }
    }

}
