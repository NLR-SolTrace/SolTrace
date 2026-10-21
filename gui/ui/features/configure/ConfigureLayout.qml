import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Dialogs
import SolTrace

InlineListEditor {
    id: root

    property var module: App.layout

    property bool has_viewed_entity: module.viewed_element.is_valid()

    model: has_viewed_entity ? module.filtered_child_model : module.filtered_root_elements_model
    emptyListText: qsTr("No elements available")
    helpText: qsTr("Scene elements combine geometry, material, and placement. Select one above, or create one to build the scene.")

    onEditingChanged: {
        App.view.editing_layout = editing
        if (!editing && App.view.mouse_mode === ViewModule.EditElement) {
            App.view.mouse_mode = ViewModule.Camera
        }
    }

    Connections {
        target: App.view
        function onEditing_layout_changed() {
            root.editing = App.view.editing_layout
        }
    }

    onItemClicked: function(index, modelData) {
        module.edited_element = modelData.entity
    }

    listHeader: RowLayout {
        Breadcrumb {
            Layout.fillWidth: true
            Layout.margins: 3

            onItemSelected: root.editing = true
            onItemDeselected: root.clearSelection()
        }

        STIconButton {
            icon: "\uf0b0"
            toolTip: qsTr("Filter Elements")
            onClicked: filter_popup.open()

            STPopup {
                id: filter_popup

                GridLayout {
                    columns: 3

                    Label {
                        Layout.fillWidth: true
                        Layout.columnSpan: 3

                        text: "Filter elements by:"
                    }

                    Label {
                        text: "Material:"
                    }

                    STClickableLabel {
                        Layout.fillWidth: true
                        property string mat_name: root.model.material_filter_name
                        text: mat_name.length ? mat_name : "All"

                        onClicked: mat_pop.open()

                        SelectItemPopup {
                            id: mat_pop
                            source_model: AppData.materials.materials_list

                            onSelectedEntity: (entity) =>
                                              root.model.material_filter = entity
                        }
                    }

                    STIconButton {
                        icon: "\uf0e2"
                        onClicked: root.model.clear_material()
                    }

                    Label {
                        text: "Geometry:"
                    }

                    STClickableLabel {
                        Layout.fillWidth: true
                        property string geo_name: root.model.geometry_filter_name
                        text: geo_name.length ? geo_name : "All"

                        onClicked: geo_pop.open()

                        SelectItemPopup {
                            id: geo_pop
                            source_model: AppData.materials.geometry_list

                            onSelectedEntity: (entity) =>
                                              root.model.geometry_filter = entity
                        }
                    }

                    STIconButton {
                        icon: "\uf0e2"
                        onClicked: root.model.clear_geometry()
                    }

                    STButton {
                        text: "Reset All"

                        onClicked: root.model.clear_all_filters()
                    }
                }
            }
        }

        STIconButton {
            id: clear_filter_button

            visible: root.model.has_filter

            icon: "\ue17b"
            toolTip: qsTr("Clear Element Filters")

            onClicked: root.model.clear_all_filters()
        }
    }

    searchBar: STSearchField {
        id: search_field
        text: root.model.name_filter

        Binding {
            target: root.model
            property: "name_filter"
            value: search_field.text
        }
    }

    actions: ColumnLayout {
        spacing: 0

        STIconButton {
            icon: "\uf002"
            toolTip: root.searchVisible ? qsTr("Hide Search")
                                            : qsTr("Search Elements")
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
            title: "New Element"
            icon: "\uf055"
            label: ""
            toolTip: title

            onCreateRequested: function(name) {
                var new_name = AppData.current_database.sanitize_element_name(name)
                var entity = AppData.current_database.add_element(new_name)
                root.module.edited_element = entity
                root.editing = true
            }
        }

        CreateNewItemButton {
            title: "New Child Element"
            icon: "\uf0fe"
            label: ""
            toolTip: title
            enabled: root.has_viewed_entity
            opacity: enabled ? 1.0 : 0.4

            onCreateRequested: function(name) {
                var new_name = AppData.current_database.sanitize_element_name(name)
                var entity = AppData.current_database.add_element(
                            new_name,
                            root.module.viewed_element
                            )
                root.module.edited_element = entity
                root.editing = true
            }
        }

        RenameItemButton {
            enabled: root.editing
            opacity: enabled ? 1.0 : 0.4
            currentName: root.module.edited_element_name
            title: qsTr("Rename Element")

            onRenameRequested: function(newName) {
                const currDb = root.module.current_database
                const sanitizedName = currDb.sanitize_element_name(newName)
                currDb.set_name_of(root.module.edited_element, sanitizedName)
            }
        }

        STIconButton {
            enabled: root.editing
            opacity: enabled ? 1.0 : 0.4
            icon: "\uf2ed"
            toolTip: qsTr("Delete Element")
            onClicked: delete_element_dialog.open()

            STDialog {
                id: delete_element_dialog
                title: qsTr("Delete Element")

                ColumnLayout {
                    anchors.fill: parent

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: qsTr("Delete this element? Any children of this element will be made top-level.")
                    }
                }

                footer: STDialogButtonBox {
                    STButton {
                        text: qsTr("Cancel")
                        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                    }

                    STButton {
                        text: qsTr("Delete")
                        Material.foreground: Material.Red
                        DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                        idle_color: App.theme.destructiveGlassColor
                        down_color: Material.color(Material.Red)

                        onClicked: {
                            root.module.delete_edited_element()
                            root.clearSelection()
                            delete_element_dialog.close()
                        }
                    }
                }
            }
        }

        STIconButton {
            icon: "\uf05b"
            toolTip: qsTr("Select Element From View")
            onClicked: App.view.mouse_mode = ViewModule.SelectElement
        }
    }

    listDelegate: STItemDelegate {
        text: itemModel ? itemModel.name : qsTr("No name")
        highlighted: itemModel
                     && itemModel.entity === root.module.edited_element

        STIconButton {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            visible: itemModel && itemModel.has_children
            icon: "\uf802"
            toolTip: qsTr("Open Children")

            onClicked: {
                root.module.viewed_element = itemModel.entity
                root.module.clear_edited_element()
                root.clearSelection()
            }
        }
    }

    detailView: ColumnLayout {
        spacing: 8

        InlineDocumentation {
            key: "configure.layout"
        }

        ElementEdit {
            Layout.fillWidth: true
        }

        STIconButton {
            Layout.alignment: Qt.AlignRight
            icon: "\uf140"
            label: qsTr("Orient Camera")
            toolTip: qsTr("Orient Camera to Element")
            enabled: root.module.instance_edit
            onClicked: simulation_scene.orient_camera_to_database_position(
                           root.module.instance_edit.global_position)
        }
    }

}
