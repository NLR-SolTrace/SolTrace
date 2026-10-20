import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SolTrace

STFormPanel {
    id: root
    property bool has_selection : false
    property var material_editor : App.materials.group_edit
    property var side_editor
    property bool singleColumn: App.view.left_panel.size <= SplitPanelData.Wide
    property var labelAlignment: (root.singleColumn ? Qt.AlignLeft : Qt.AlignRight) | Qt.AlignVCenter

    // =========================================================================

    STFormRow {
        label: qsTr("Preset")
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STButton {
            text: qsTr("Load")

            Layout.fillWidth: true

            onClicked: {
                preset_popup.open()
            }

            STPopup {
                id: preset_popup

                RowLayout {
                    Layout.fillWidth: true

                    STButton {
                        Layout.fillWidth: true
                        text: qsTr("Absorber")
                        onClicked: {
                            if (root.side_editor) {
                                root.side_editor.set_ideal_absorption()
                            }
                            preset_popup.close()
                        }
                    }

                    STButton {
                        Layout.fillWidth: true
                        text: qsTr("Reflector")
                        onClicked: {
                            if (root.side_editor) {
                                root.side_editor.set_ideal_reflection()
                            }
                            preset_popup.close()
                        }
                    }

                    STButton {
                        Layout.fillWidth: true
                        text: qsTr("Transmitter")
                        onClicked: {
                            if (root.side_editor) {
                                root.side_editor.set_ideal_transmission()
                            }
                            preset_popup.close()
                        }
                    }
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("Optical behavior")
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STComboBox {
            id: interactionCombo
            Layout.fillWidth: true
            model: App.materials.material_edit.interaction_type_model
            textRole: "name"
            valueRole: "value"

            currentValue: root.side_editor.interaction_type

            onActivated: {
                if (root.side_editor) {
                    root.side_editor.interaction_type = currentValue
                }
            }
        }
    }



    // =========================================================================

    STFormRow {
        label: qsTr("Distribution")
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STComboBox {
            id: distributionCombo
            Layout.fillWidth: true
            model: App.materials.material_edit.distribution_type_model
            textRole: "name"
            valueRole: "value"

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
        label: qsTr("Reflectance (ρ)")
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0.0
            to: 1.0
            decimals: 4
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.reflectivity : 0
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.reflectivity = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("Transmittance (τ)")
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0
            to: 1.0
            decimals: 4
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.transmissivity : 0
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.transmissivity = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("<em>n</em> Front")
        labelTextFormat: Label.RichText
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 1.0
            to: 5.0
            decimals: 4
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.refraction_index_front : 1
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.refraction_index_front = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("<em>n</em> Back")
        labelTextFormat: Label.RichText
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 1
            to: 5
            decimals: 4
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.refraction_index_back : 1
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.refraction_index_back = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("σ<sub>slope</sub> (mrad)")
        labelTextFormat: Label.RichText
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0
            to: 1000
            decimals: 3
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.slope_error : 0
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.slope_error = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("σ<sub>spec</sub> (mrad)")
        labelTextFormat: Label.RichText
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STDoubleSpinBox {
            Layout.fillWidth: true
            from: 0
            to: 1000
            decimals: 3
            stepSize: 0.01
            value: root.side_editor ? root.side_editor.specularity_error : 0
            onValueModified: {
                if (root.side_editor) {
                    root.side_editor.specularity_error = value
                }
            }
        }
    }

    // =========================================================================

    STFormRow {
        label: qsTr("Error Type")
        visible: false
        singleColumn: root.singleColumn
        labelAlignment: root.labelAlignment

        STComboBox {
            Layout.fillWidth: true
            model: App.materials.material_edit.distribution_type_model
            textRole: "name"
            valueRole: "value"
            currentValue: root.side_editor.error_distribution_type
            enabled: count > 0
            onActivated: {
                if (root.side_editor) {
                    root.side_editor.error_distribution_type = currentValue
                }
            }
        }
    }

    // =========================================================================

    // RowLayout {
    //     Layout.columnSpan: 2
    //     Layout.fillWidth: true
    //     spacing: 12

    //     Button {
    //         Layout.fillWidth: true
    //         text: "Angular Reflectance"
    //         enabled: false
    //     }

    //     Button {
    //         Layout.fillWidth: true
    //         text: "Angular Transmittance"
    //         enabled: false
    //     }
    // }
}
