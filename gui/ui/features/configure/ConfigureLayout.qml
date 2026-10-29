import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import SolTrace

InlineTreeEditor {
    id: root

    property var module: App.layout

    model: module.filtered_element_tree_model
    selectedEntity: module.edited_element
    emptyListText: qsTr("No elements available")
    helpText: qsTr("Scene elements combine geometry, material, and placement. Select one above, or create one to build the scene.")

    function synchronizeSelection() {
        const entity = module.edited_element
        if (!entity || !entity.is_valid()) {
            root.editing = false
            return
        }

        root.editing = true
        root.revealEntity(entity)
    }

    function refreshTree() {
        if (root.model.has_filter) root.expandAll()
        Qt.callLater(root.synchronizeSelection)
    }

    onModelChanged: Qt.callLater(root.synchronizeSelection)

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

    Connections {
        target: root.module

        function onEdited_element_changed() {
            Qt.callLater(root.synchronizeSelection)
        }
    }

    Connections {
        target: root.model
        ignoreUnknownSignals: true

        function onLayoutChanged() {
            root.refreshTree()
        }

        function onModelReset() {
            root.refreshTree()
        }

        function onRowsInserted() {
            root.refreshTree()
        }

        function onRowsRemoved() {
            root.refreshTree()
        }
    }

    onItemClicked: function(entity) {
        module.edited_element = entity
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

        STIconButton {
            icon: root.model.has_filter ? "\ue17b" : "\uf0b0"
            toolTip: root.model.has_filter ? qsTr("Edit or Clear Element Filters")
                                           : qsTr("Filter Elements")
            onClicked: filter_popup.open()

            STPopup {
                id: filter_popup

                GridLayout {
                    columns: 3

                    Label {
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        text: qsTr("Filter elements by:")
                    }

                    Label { text: qsTr("Material:") }

                    STClickableLabel {
                        Layout.fillWidth: true
                        property string mat_name: root.model.material_filter_name
                        text: mat_name.length ? mat_name : qsTr("All")
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
                        enabled: root.model.material_filter.is_valid()
                        opacity: enabled ? 1.0 : 0.4
                        toolTip: qsTr("Clear Material Filter")
                        onClicked: root.model.clear_material()
                    }

                    Label { text: qsTr("Geometry:") }

                    STClickableLabel {
                        Layout.fillWidth: true
                        property string geo_name: root.model.geometry_filter_name
                        text: geo_name.length ? geo_name : qsTr("All")
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
                        enabled: root.model.geometry_filter.is_valid()
                        opacity: enabled ? 1.0 : 0.4
                        toolTip: qsTr("Clear Geometry Filter")
                        onClicked: root.model.clear_geometry()
                    }

                    STButton {
                        text: qsTr("Reset All")
                        enabled: root.model.has_filter
                        onClicked: root.model.clear_all_filters()
                    }
                }
            }
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
            enabled: root.module.edited_element.is_valid()
            opacity: enabled ? 1.0 : 0.4

            onCreateRequested: function(name) {
                var new_name = AppData.current_database.sanitize_element_name(name)
                var entity = AppData.current_database.add_element(
                            new_name,
                            root.module.edited_element
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

    detailView: ColumnLayout {
        spacing: 8

        InlineDocumentation {
            key: "configure.layout"
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

        ElementEdit {
            Layout.fillWidth: true
        }
    }

}
