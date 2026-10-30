import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

FilteredInlineListEditor {
    id: root

    source_model: App.materials.geometry_list
    emptyListText: qsTr("No geometries available")
    helpText: qsTr("Geometries define reusable element shapes. Select one above, or create one to assign to scene elements.")

    onEditingChanged: {
        App.view.editing_geometry = editing
    }

    Connections {
        target: App.view
        function onEditing_geometry_changed() {
            root.editing = App.view.editing_geometry
        }
    }

    Connections {
        target: App.materials
        function onCurrent_geometry_changed() {
            root.selectSourceIndex(
                        root.source_model.index_of(App.materials.current_geometry))
        }
    }

    onItemClicked: function(index, modelData) {
        App.materials.current_geometry = modelData.entity
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
                                            : qsTr("Search Geometries")
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
            title: qsTr("New Geometry")
            label: ""
            toolTip: title

            onCreateRequested: function(name) {
                var new_name = AppData.current_database.sanitize_geometry_name(name)
                App.materials.current_geometry =
                        AppData.current_database.add_geometry_group(new_name)
            }
        }

        RenameItemButton {
            enabled: root.editing
            opacity: enabled ? 1.0 : 0.4
            currentName: App.materials.current_geometry_name
            title: qsTr("Rename Geometry")

            onRenameRequested: function(newName) {
                const mats = App.materials
                const currDb = mats.current_database
                const sanitizedName = currDb.sanitize_geometry_name(newName)
                currDb.set_name_of(mats.current_geometry, sanitizedName)
            }
        }

        DeleteItemButton {
            id: delete_button

            enabled: root.editing
            opacity: enabled ? 1.0 : 0.4
            label: ""
            toolTip: title
            title: qsTr("Delete Geometry")
            itemType: qsTr("geometry")
            replacementModel: root.source_model
            toDelete: AppData.materials.current_geometry

            onBeforeOpened: {
                isDangerous = AppData.current_database.geometry_use_count(
                            delete_button.toDelete)
            }

            onDeleteRequested: {
                AppData.current_database.delete_geometry_group(
                            delete_button.toDelete)
                root.clearSelection()
            }

            onDeleteReassignRequested: function(entity) {
                AppData.current_database.delete_geometry_group(
                            delete_button.toDelete, entity)
                root.clearSelection()
            }
        }

        STIconButton {
            icon: "\uf05b"
            toolTip: qsTr("Select Geometry From View")
            onClicked: App.view.mouse_mode = ViewModule.SelectGeometry
        }
    }

    listDelegate: STItemDelegate {
        text: itemModel ? itemModel.name : qsTr("No name")
        highlighted: isCurrent
    }

    detailView: ColumnLayout {
        spacing: 8

        InlineDocumentation {
            key: "configure.geometry"
        }

        SurfacePreviewScene {
            id: preview_scene
            Layout.fillWidth: true

            property bool show_expand: false

            Layout.preferredHeight: show_expand ? 300 : 148

            property bool no_geometry:
                App.materials.geometry_edit.surface_geometry.vertex_count === 0

            Row {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 3
                spacing: 3

                STIconButton {
                    icon: preview_scene.show_expand ? "\uf066" : "\uf065"
                    toolTip: preview_scene.show_expand
                             ? qsTr("Collapse preview")
                             : qsTr("Expand preview")
                    onClicked: preview_scene.show_expand = !preview_scene.show_expand
                }

                STIconButton {
                    icon: "\uf0b2"
                    toolTip: qsTr("Fit geometry in view")
                    enabled: !preview_scene.no_geometry
                    onClicked: preview_scene.fitGeometry()
                }
            }

            RowLayout {
                visible: parent.no_geometry

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 3

                Label {
                    text: "\uf071"

                    font.family: "Font Awesome 7 Free"

                    color: Material.color(Material.Yellow)
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("No geometry for this configuration.")

                    elide: Label.ElideRight

                    color: Material.color(Material.Yellow)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? implicitHeight : 0
            implicitHeight: geometry_errors.implicitHeight + 12
            visible: error_repeater.count > 0
            radius: 8
            color: Qt.alpha(Material.color(Material.Yellow), 0.08)
            border.color: Qt.alpha(Material.color(Material.Yellow), 0.45)

            ColumnLayout {
                id: geometry_errors
                anchors.fill: parent
                anchors.margins: 6
                spacing: 4

                Repeater {
                    id: error_repeater
                    model: App.materials.geometry_edit.geometry_error_model

                    delegate: RowLayout {
                        required property string display
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            text: "\uf071"
                            font.family: "Font Awesome 7 Free"
                            color: Material.color(Material.Yellow)
                        }

                        Label {
                            Layout.fillWidth: true
                            text: display
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                            color: Material.color(Material.Yellow)
                        }
                    }
                }
            }
        }

        GeometryProperties {
            Layout.fillWidth: true
        }
    }

}
