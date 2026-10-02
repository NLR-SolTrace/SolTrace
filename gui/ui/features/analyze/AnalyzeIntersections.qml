import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

Flickable {
    id: root
    property var left_panel_size: App.view.left_panel.size
    property var intersections_module : AppData.intersections

    property var ray_geom: AppData.intersections.ray_geometry

    property bool is_selected_ray_valid: ray_geom.selected_ray_id >= 0

    function formatRayCount(count) {
        return Number(count).toLocaleString(Qt.locale(), "f", 0) + " rays"
    }

    function formatDetail(value, suffix) {
        if (!isFinite(value)) {
            return "--"
        }

        return Number(value).toLocaleString(Qt.locale(), "G", 6)
                + (suffix.length ? " " + suffix : "")
    }

    function formatPowerPerRay() {
        const ray_area_weight = root.intersections_module.ray_area_weight
        const dni = AppData.flux.dni
        if (!isFinite(ray_area_weight) || ray_area_weight <= 0
                || !isFinite(dni) || dni < 0) {
            return "--"
        }

        return root.formatDetail(dni * ray_area_weight, "W")
    }

    function visibleRayCount() {
        const available = ray_geom.available_rays
        return Math.round(available *
                          ray_geom.show_percent / 100.0)
    }

    function setVisibleRayCount(count) {
        const available = ray_geom.available_rays
        ray_geom.show_percent =
                available > 0 ? count * 100.0 / available : 0.0
    }

    contentWidth: width
    contentHeight: content_column.implicitHeight
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: STScrollBar { }



    ColumnLayout {
        id: content_column
        width: root.width

        InlineDocumentation {
            key: "analyze.intersections"
        }

        Label {
            Layout.fillWidth: true
            text: root.formatRayCount(
                      root.ray_geom.available_rays)
            opacity: 0.75
            horizontalAlignment: Text.AlignHCenter
        }

        STFormPanel {
            collapsible: true
            title: "Trace Details"
            visible: root.intersections_module.has_results

            Label {
                Layout.fillWidth: true
                //Layout.columnSpan: ppi_row.singleColumn ? 1 : 2
                text: "Power per ray uses the flux-map DNI of "
                      + root.formatDetail(AppData.flux.dni, "W/m²") + "."
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                opacity: 0.75
            }

            STFormRow {
                label: "Sun Rays Traced"

                Label {
                    Layout.fillWidth: true
                    text: Number(root.intersections_module.sun_ray_count)
                              .toLocaleString(Qt.locale(), "f", 0)
                }
            }

            STFormRow {
                label: "Sun Plane Box"

                Label {
                    Layout.fillWidth: true
                    text: root.formatDetail(
                              root.intersections_module.sun_plane_width, "m")
                          + " × "
                          + root.formatDetail(
                              root.intersections_module.sun_plane_height, "m")
                }
            }

            STFormRow {
                id: ppi_row
                label: "Power per Ray"

                Label {
                    Layout.fillWidth: true
                    text: root.formatPowerPerRay()
                }
            }


        }

        STFormPanel {
            collapsible: true
            title: "Ray Visibility"


            STSwitch {
                Layout.fillWidth: true

                text: "Toggle Interactions"
                checked: AppData.view.show_intersections

                onToggled: AppData.view.show_intersections = checked
            }

            STSwitch {
                Layout.fillWidth: true

                text: "Show Geometry"
                checked: root.intersections_module.show_geometry

                onToggled: root.intersections_module.show_geometry = checked
            }

            STFormRow {
                label: "Show as"

                STComboBox {
                    id: render_mode
                    Layout.fillWidth: true
                    currentIndex: root.ray_geom.isect_mode === RayGeometry.Point
                                  ? 1 : 0
                    model: ["Lines", "Points"]

                    onCurrentIndexChanged: {
                        root.ray_geom.isect_mode = currentIndex === 0
                                ? RayGeometry.Line
                                : RayGeometry.Point
                    }

                    property bool point_mode: currentIndex === 1
                }
            }

            STFormRow {
                label: "Point Size"
                visible: render_mode.point_mode

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 10
                    value: AppData.view.point_size
                    onValueChanged: {
                        AppData.view.point_size = value
                    }
                }
            }

            STFormRow {
                label: "Filter element"

                RowLayout {
                    Layout.fillWidth: true

                    STButton {
                        Layout.fillWidth: true
                        text: root.ray_geom.entity_filter.is_valid()
                              ? root.ray_geom.entity_filter_name
                              : "All Elements"
                        left_text_icon: "\uf03a"

                        onClicked: entity_filter_pop.open()

                        SelectItemPopup {
                            id: entity_filter_pop
                            source_model: root.intersections_module.entity_model

                            onSelectedEntity: (entity) => {
                                root.ray_geom.select_entity_filter(entity)
                            }
                        }
                        Layout.rightMargin: 4
                    }

                    STIconButton {
                        icon: "\uf245"
                        toolTip: "Pick element from view"

                        onClicked: {
                            App.view.simulation_content_view = true
                            App.view.mouse_mode = ViewModule.SelectRayFilterElement
                        }

                        Layout.rightMargin: 4
                    }

                    STIconButton {
                        icon: "\uf057"
                        toolTip: "Clear element filter"
                        enabled: root.ray_geom.entity_filter.is_valid()

                        onClicked: root.ray_geom.clear_entity_filter()
                    }
                }
            }

            STFormRow {
                label: "Filter types"

                STButton {
                    Layout.fillWidth: true
                    text: root.ray_geom.event_include.length > 0
                          ? root.ray_geom.event_include.join(", ")
                          : "None"

                    onClicked: ray_filter_popup.open_with(
                                   root.ray_geom.event_include)

                    RayFilterPopup {
                        id: ray_filter_popup

                        onModified: function(filter) {
                            root.ray_geom.event_include = filter
                        }
                    }
                }
            }

            STFormRow {
                label: "Color mode"

                STComboBox {
                    Layout.fillWidth: true

                    currentValue: root.ray_geom.texture_mode

                    model: [
                        {value: RayGeometry.SolidColor, text: "Solid Color"},
                        {value: RayGeometry.Length, text: "Length"},
                        {value: RayGeometry.Segment, text: "Segment"},
                        {value: RayGeometry.Element, text: "Element"},
                    ]

                    textRole: "text"
                    valueRole: "value"

                    onActivated: root.ray_geom.texture_mode = currentValue
                }
            }

            ColorPickerField {
                id: rayColorPicker
                Layout.fillWidth: true
                color: AppData.view.ray_color
                label: "Ray Color"
                visible: root.ray_geom.texture_mode === RayGeometry.SolidColor
                onUpdated: AppData.view.ray_color = rayColorPicker.color
            }

            STFormRow {
                label: "Visible rays percent"

                STDoubleSpinBox {
                    from: 0.0
                    to: 100.0
                    value: root.ray_geom.show_percent
                    stepSize: 1.0
                    decimals: 0
                    Layout.fillWidth: true
                    onValueModified: {
                        root.ray_geom.show_percent = value
                    }
                }
            }

            STFormRow {
                label: "Visible rays"

                STDoubleSpinBox {
                    from: 0.0
                    to: root.ray_geom.available_rays
                    value: root.visibleRayCount()
                    stepSize: 1000
                    decimals: 0
                    Layout.fillWidth: true
                    onValueModified: root.setVisibleRayCount(value)
                }
            }

            STFormRow {
                label: "Cutoff radius"

                STDoubleSpinBox {
                    Layout.fillWidth: true
                    from: 0.0
                    to: Infinity
                    value: root.ray_geom.max_ray_distance
                    stepSize: 100.0
                    decimals: 0
                    onValueModified: root.ray_geom.max_ray_distance = value
                }
            }

            STFormRow {
                label: "Opacity percent"

                STDoubleSpinBox {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 100.0
                    value: AppData.view.intersection_opacity * 100.0
                    stepSize: 5.0
                    decimals: 0
                    onValueModified: {
                        AppData.view.intersection_opacity = value / 100.0
                    }
                }
            }

            STPropertySeparator {
                title: "Selection"
            }

            STFormRow {
                label: "Selected Ray ID"
                visible: root.is_selected_ray_valid

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        text: root.ray_geom.selected_ray_id
                    }

                    STIconButton {
                        icon: "\uf057"

                        onClicked: root.ray_geom.selected_ray_id = -1
                    }
                }
            }

            ListView {
                id: interaction_list_view
                Layout.fillWidth: true
                Layout.columnSpan: parent.columns
                Layout.margins: 6

                clip: true

                spacing: 3

                model: root.ray_geom.selected_ray_interactions

                Layout.preferredHeight: count > 0
                                        ? (headerItem ? headerItem.implicitHeight : 0)
                                          + Math.min(count, 5) * delegate_height
                                          + Math.max(0, Math.min(count, 5) - 1) * spacing
                                        : 0

                property int delegate_height: 42

                ScrollBar.vertical: STScrollBar { }

                header: Label {
                    text: "Interactions"
                }

                delegate: Rectangle {
                    id: id_root
                    required property bool     has_entity
                    required property var      entity
                    required property string   interaction_type
                    required property vector3d location

                    radius: 6

                    width: ListView.view.width
                    height: interaction_content.implicitHeight + 12

                    color: Material.dropShadowColor

                    ColumnLayout {
                        id: interaction_content
                        anchors.fill: parent

                        anchors.margins: 5

                        Label {
                            Layout.fillWidth: true
                            text: id_root.interaction_type
                            font.bold: true
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                Layout.fillWidth: true
                                Layout.horizontalStretchFactor: 0
                                text: Number(id_root.location.x).toLocaleString(Qt.locale(), 'f', 3)
                                color: Qt.tint(Material.foreground,
                                               Qt.alpha(Material.color(Material.Red), 0.25))
                            }

                            Label {
                                text: " "
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.horizontalStretchFactor: 0
                                text: Number(id_root.location.y).toLocaleString(Qt.locale(), 'f', 3)
                                color: Qt.tint(Material.foreground,
                                               Qt.alpha(Material.color(Material.Green), 0.25))
                            }

                            Label {
                                text: " "
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.horizontalStretchFactor: 0
                                text: Number(id_root.location.z).toLocaleString(Qt.locale(), 'f', 3)
                                color: Qt.tint(Material.foreground,
                                               Qt.alpha(Material.color(Material.Blue), 0.25))
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.horizontalStretchFactor: 1
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.horizontalStretchFactor: 0
                                horizontalAlignment: Qt.AlignRight
                                visible: id_root.has_entity
                                text: "@ Entity " + id_root.entity
                            }
                        }
                    }
                }
            }

            STButton {
                Layout.fillWidth: true
                text: "Pick Ray From View"
                left_text_icon: "\uf05b"
                onClicked: App.view.mouse_mode = ViewModule.PickRay
            }

        }
    }
}
