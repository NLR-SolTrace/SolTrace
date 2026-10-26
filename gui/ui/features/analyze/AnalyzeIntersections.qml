import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STScrollView {
    id: root
    property var intersections_module : AppData.intersections

    property var ray_geom: AppData.intersections.ray_geometry

    property bool is_selected_ray_valid: ray_geom.selected_ray_id >= 0

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

    function setVisibleRayPercent(percent) {
        ray_geom.show_percent = percent
    }

    function formatInteractionTypes(types) {
        if (!types || types.length === 0) {
            return "None"
        }
        if (types.length === ray_filter_popup.ray_type_model.length) {
            return "All"
        }

        const names = []
        for (let i = 0; i < types.length; ++i) {
            const value = String(types[i])
            names.push(value.charAt(0).toUpperCase() + value.slice(1))
        }
        return names.length <= 2 ? names.join(", ")
                                 : names.length + " selected"
    }

    contentWidth: availableWidth

    ColumnLayout {
        id: content_column
        width: root.availableWidth

        InlineDocumentation {
            key: "analyze.intersections"
        }

        Label {
            Layout.fillWidth: true
            visible: !root.intersections_module.has_results
            text: "Select a simulation result to inspect its rays."
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.7
            Layout.margins: 12
        }

        STFormPanel {
            collapsible: true
            title: "Display"
            documentationKey: "analyze.intersections.display"
            visible: root.intersections_module.has_results


            STSwitch {
                Layout.fillWidth: true

                text: "Show Rays"
                checked: AppData.view.show_intersections

                ToolTip.visible: hovered
                ToolTip.text: "Show or hide traced rays and interaction points in the viewport."

                onToggled: AppData.view.show_intersections = checked
            }

            STSwitch {
                Layout.fillWidth: true

                text: "Show Geometry"
                checked: root.intersections_module.show_geometry

                ToolTip.visible: hovered
                ToolTip.text: "Show the traced scene geometry behind the rays."

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

                    ToolTip.visible: hovered
                    ToolTip.text: "Draw complete ray paths or individual interaction points."

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

                    ToolTip.visible: hovered
                    ToolTip.text: currentValue === RayGeometry.Length
                                      ? "Color each path by distance along the ray."
                                      : currentValue === RayGeometry.Segment
                                        ? "Alternate colors between consecutive ray segments."
                                        : currentValue === RayGeometry.Element
                                          ? "Color interactions according to the element they hit."
                                          : "Use one color for every ray."

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
                label: "Visible Rays"

                ColumnLayout {
                    Layout.fillWidth: true

                    STDoubleSpinBox {
                        from: 0.0
                        to: root.ray_geom.available_rays
                        value: root.visibleRayCount()
                        stepSize: 1000
                        decimals: 0
                        Layout.fillWidth: true
                        onValueModified: root.setVisibleRayCount(value)

                        ToolTip.visible: hovered
                        ToolTip.text: "Limit how many recorded rays are drawn."
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Item { Layout.fillWidth: true }

                        STIconButton {
                            label: "10%"
                            toolTip: "Show 10% of recorded rays"
                            onClicked: root.setVisibleRayPercent(10)
                        }

                        STIconButton {
                            label: "50%"
                            toolTip: "Show 50% of recorded rays"
                            onClicked: root.setVisibleRayPercent(50)
                        }

                        STIconButton {
                            label: "100%"
                            toolTip: "Show all recorded rays"
                            onClicked: root.setVisibleRayPercent(100)
                        }
                    }
                }
            }

            STFormRow {
                label: "Maximum Ray Distance"

                STDoubleSpinBox {
                    Layout.fillWidth: true
                    from: 0.0
                    to: Infinity
                    value: root.ray_geom.max_ray_distance
                    stepSize: 100.0
                    decimals: 0
                    onValueModified: root.ray_geom.max_ray_distance = value

                    ToolTip.visible: hovered
                    ToolTip.text: "Clip ray paths beyond this distance from the scene origin."
                }
            }

            STFormRow {
                label: "Ray Opacity"

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

                    ToolTip.visible: hovered
                    ToolTip.text: "Set ray visibility from 0 to 100 percent."
                }
            }

        }

        STFormPanel {
            collapsible: true
            title: "Filters"
            documentationKey: "analyze.intersections.filters"
            visible: root.intersections_module.has_results

            STFormRow {
                label: "Element"

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
                        toolTip: "Pick filter element from the viewport"

                        onClicked: {
                            App.view.simulation_content_view = true
                            App.view.mouse_mode = ViewModule.SelectRayFilterElement
                        }

                        Layout.rightMargin: 4
                    }

                    STIconButton {
                        icon: "\uf057"
                        toolTip: "Clear element and side filtering"
                        enabled: root.ray_geom.entity_filter.is_valid()

                        onClicked: root.ray_geom.clear_entity_filter()
                    }
                }
            }

            STFormRow {
                label: "Hit Side"

                STComboBox {
                    Layout.fillWidth: true
                    enabled: root.ray_geom.entity_filter.is_valid()
                    currentValue: root.ray_geom.surface_side_filter
                    textRole: "text"
                    valueRole: "value"

                    model: [
                        { text: "Either", value: RayGeometry.Either },
                        { text: "Front", value: RayGeometry.Front },
                        { text: "Back", value: RayGeometry.Back }
                    ]

                    ToolTip.visible: hovered
                    ToolTip.text: enabled
                                      ? "Filter by the side on which rays arrive at the selected element."
                                      : "Select an element to enable side filtering."

                    onActivated: root.ray_geom.surface_side_filter = currentValue
                }
            }

            STFormRow {
                label: "Interaction Types"

                STButton {
                    Layout.fillWidth: true
                    text: root.formatInteractionTypes(
                              root.ray_geom.event_include)

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
        }

        STFormPanel {
            collapsible: true
            title: "Selected Ray"
            documentationKey: "analyze.intersections.selection"
            visible: root.intersections_module.has_results

            STButton {
                Layout.fillWidth: true
                text: root.is_selected_ray_valid
                      ? "Pick Another Ray From View"
                      : "Pick Ray From View"
                left_text_icon: "\uf05b"
                onClicked: {
                    App.view.simulation_content_view = true
                    App.view.mouse_mode = ViewModule.PickRay
                }
            }

            Label {
                Layout.fillWidth: true
                visible: !root.is_selected_ray_valid
                text: "Pick a ray in the viewport to inspect its interaction history."
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.7
                Layout.margins: 6
            }

            STFormRow {
                label: "Ray ID"
                visible: root.is_selected_ray_valid

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        text: root.ray_geom.selected_ray_id
                    }

                    STIconButton {
                        icon: "\uf057"
                        toolTip: "Clear selected ray"
                        onClicked: root.ray_geom.selected_ray_id = -1
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: root.is_selected_ray_valid
                text: "Interactions"
                font.bold: true
            }

            Repeater {
                model: root.ray_geom.selected_ray_interactions

                delegate: Rectangle {
                    id: interaction_delegate
                    required property bool     has_entity
                    required property string   entity_name
                    required property string   interaction_type
                    required property vector3d location

                    Layout.fillWidth: true
                    implicitHeight: interaction_content.implicitHeight + 12
                    radius: 6
                    color: Material.dropShadowColor

                    ColumnLayout {
                        id: interaction_content
                        anchors.fill: parent
                        anchors.margins: 5

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                text: interaction_delegate.interaction_type
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Label {
                                visible: interaction_delegate.has_entity
                                Layout.maximumWidth: interaction_content.width * 0.6
                                text: interaction_delegate.entity_name
                                horizontalAlignment: Qt.AlignRight
                                elide: Text.ElideRight
                                opacity: 0.75
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "X " + Number(interaction_delegate.location.x)
                                      .toLocaleString(Qt.locale(), "f", 3)
                                color: Qt.tint(Material.foreground,
                                               Qt.alpha(Material.color(Material.Red), 0.25))
                            }

                            Label {
                                text: "Y " + Number(interaction_delegate.location.y)
                                      .toLocaleString(Qt.locale(), "f", 3)
                                color: Qt.tint(Material.foreground,
                                               Qt.alpha(Material.color(Material.Green), 0.25))
                            }

                            Label {
                                text: "Z " + Number(interaction_delegate.location.z)
                                      .toLocaleString(Qt.locale(), "f", 3)
                                color: Qt.tint(Material.foreground,
                                               Qt.alpha(Material.color(Material.Blue), 0.25))
                            }

                            Item { Layout.fillWidth: true }
                        }
                    }
                }
            }
        }
    }
}
