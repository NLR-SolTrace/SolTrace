import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

FilteredInlineListEditor {
    id: root

    source_model: App.materials.materials_list

    emptyListText: qsTr("No materials available")
    helpText: qsTr("Materials define optical properties. Select one above, or create one to assign to scene elements.")

    onEditingChanged: {
        App.view.editing_material = editing
    }

    Connections {
        target: App.view
        function onEditing_material_changed() {
            root.editing = App.view.editing_material
        }
    }

    Connections {
        target: App.materials
        function onCurrent_material_changed() {
            root.selectSourceIndex(
                        root.source_model.index_of(App.materials.current_material))
        }
    }

    onItemClicked: function(index, modelData) {
        // if (App.db) App.db.clear_selection()
        App.materials.current_material = modelData.entity
        // if (App.db) App.db.select_all_with_material(modelData.entity)
    }

    searchBar: STSearchField {
        id: search_field

        Binding {
            target: root
            property: "filterText"
            value: search_field.text
        }
    }

    actions: ColumnLayout {
        spacing: 0

        STIconButton {
            icon: "\uf002"
            toolTip: root.searchVisible ? qsTr("Hide Search")
                                            : qsTr("Search Materials")
            onClicked: root.searchVisible = !root.searchVisible
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.topMargin: 3
            Layout.bottomMargin: 3
            color: Material.dividerColor
        }

        CreateNewItemButton {
            title: qsTr("New Material")
            label: ""
            toolTip: title

            onCreateRequested: function(name) {
                var new_name = AppData.current_database.sanitize_material_name(name)
                App.materials.current_material =
                        AppData.current_database.add_material_group(new_name)
            }
        }

        RenameItemButton {
            enabled: root.editing
            opacity: enabled ? 1.0 : 0.4
            currentName: App.materials.current_material_name
            title: qsTr("Rename Material")

            onRenameRequested: function(newName) {
                const mats = App.materials
                const currDb = mats.current_database
                const sanitizedName = currDb.sanitize_material_name(newName)
                currDb.set_name_of(mats.current_material, sanitizedName)
            }
        }

        DeleteItemButton {
            id: delete_button

            enabled: root.editing
            opacity: enabled ? 1.0 : 0.4
            label: ""
            toolTip: title
            title: qsTr("Delete Material")
            itemType: qsTr("material")
            replacementModel: root.source_model
            toDelete: AppData.materials.current_material

            onBeforeOpened: {
                isDangerous = AppData.current_database.material_use_count(
                            delete_button.toDelete)
            }

            onDeleteRequested: {
                AppData.current_database.delete_material_group(
                            delete_button.toDelete)
                root.clearSelection()
            }

            onDeleteReassignRequested: function(entity) {
                AppData.current_database.delete_material_group(
                            delete_button.toDelete, entity)
                root.clearSelection()
            }
        }

        STIconButton {
            icon: "\uf05b"
            toolTip: qsTr("Select Material From View")
            onClicked: App.view.mouse_mode = ViewModule.SelectMaterial
        }
    }

    listDelegate: STItemDelegate {
        text: itemModel ? itemModel.name : qsTr("No name")
        highlighted: isCurrent
    }

    detailView: ColumnLayout {
        spacing: 8

        InlineDocumentation {
            key: "configure.materials"
        }

        SurfacePropertyGraphic {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 148

                    property var face: bar.currentIndex == 0
                                       ? App.materials.material_edit.front_editor
                                       : App.materials.material_edit.back_editor

                    reflectance: face.reflectivity
                    transmittance: face.transmissivity
                    nFront: face.refraction_index_front
                    nBack: face.refraction_index_back
                    incidentFromBack: bar.currentIndex == 1
                    slopeErrorMrad: face.slope_error
                    specularityErrorMrad: face.specularity_error
                }

        STFormPanel {
                    id: refraction_indices
                    title: qsTr("Refraction indices")

                    readonly property var editor:
                        App.materials.material_edit.front_editor

                    STFormRow {
                        label: qsTr("Front medium (<em>n</em>)")
                        labelTextFormat: Label.RichText

                        STDoubleSpinBox {
                            Layout.fillWidth: true
                            from: 1.0
                            to: 5.0
                            decimals: 4
                            stepSize: 0.01
                            value: refraction_indices.editor
                                   ? refraction_indices.editor.refraction_index_front
                                   : 1.0
                            onValueModified: {
                                if (refraction_indices.editor) {
                                    refraction_indices.editor.refraction_index_front
                                            = value
                                }
                            }
                        }
                    }

                    STFormRow {
                        label: qsTr("Back medium (<em>n</em>)")
                        labelTextFormat: Label.RichText

                        STDoubleSpinBox {
                            Layout.fillWidth: true
                            from: 1.0
                            to: 5.0
                            decimals: 4
                            stepSize: 0.01
                            value: refraction_indices.editor
                                   ? refraction_indices.editor.refraction_index_back
                                   : 1.0
                            onValueModified: {
                                if (refraction_indices.editor) {
                                    refraction_indices.editor.refraction_index_back
                                        = value
                                }
                            }
                        }
                    }
                }

        STComboBar {
                    id: bar
                    Layout.fillWidth: true
                    model: [qsTr("Front"), qsTr("Back")]
                }

        SwipeView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: currentItem ? currentItem.implicitHeight : 0
                    interactive: false
                    clip: true
                    currentIndex: bar.currentIndex

                    MaterialOpticals {
                        collapsed: false
                        Layout.fillWidth: true
                        side_editor: App.materials.material_edit.front_editor
                    }

                    MaterialOpticals {
                        collapsed: false
                        Layout.fillWidth: true
                        side_editor: App.materials.material_edit.back_editor
                    }
        }
    }

}
