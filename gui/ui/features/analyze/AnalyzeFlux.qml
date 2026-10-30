import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Dialogs

import SolTrace

STScrollView {
    id: root
    property var flux_module : AppData.flux

    function formatCoordinate(value) {
        if (!isFinite(value)) {
            return "--"
        }

        return Number(value).toFixed(3)
    }

    function formatCentroid(centroid) {
        if (!centroid) {
            return "--"
        }

        return "(" + formatCoordinate(centroid.x) + ", "
                   + formatCoordinate(centroid.y) + ", "
                   + formatCoordinate(centroid.z) + ")"
    }

    function formatStatistic(value, suffix = "") {
        let suffix_part = suffix.length ? " " + suffix : ""
        return Number(value).toLocaleString(Qt.locale(), 'G', 6) + suffix_part
    }

    function contributionSummary() {
        const filters = root.flux_module.pending_flux_maps
        const include_direct = filters.include_direct_sun_rays
        const terminal_only = filters.terminal_interactions_only

        if (include_direct && !terminal_only) {
            return "All Contributions"
        }
        if (!include_direct && terminal_only) {
            return "Indirect · Terminating"
        }
        return include_direct ? "Terminating Only" : "Indirect Only"
    }

    function closestFluxGridResolution(size) {
        const resolutions = [128, 256, 512, 1024, 2048, 4096]
        const target = Math.max(size.width, size.height)
        let closest = resolutions[0]

        for (const resolution of resolutions) {
            if (Math.abs(resolution - target)
                    < Math.abs(closest - target)) {
                closest = resolution
            }
        }

        return closest
    }

    contentWidth: availableWidth

    ColumnLayout {
        id: content_column
        width: root.availableWidth

        InlineDocumentation {
            key: "analyze.flux"
        }

        Label {
            Layout.fillWidth: true
            visible: !root.flux_module.has_results
            text: "Select a simulation result to compute or inspect flux maps."
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.7
            Layout.margins: 12
        }

        STFormPanel {
            collapsible: true
            title: "Current Flux Map"
            documentationKey: "analyze.flux.display"

            visible: root.flux_module.has_results
                     && computed_map_combo.count > 0

            STFormRow {
                label: "Computed Map"
                visible: computed_map_combo.count > 0

                STComboBox {
                    id: computed_map_combo
                    Layout.fillWidth: true

                    model: AppData.flux.computed_maps_model
                    textRole: "name"
                    currentIndex: Math.max(
                                      0,
                                      AppData.flux.computed_maps_model.index_of(
                                          AppData.flux.current_map_entity))

                    onActivated: (index) => {
                        AppData.flux.select_map(
                                    AppData.flux.computed_maps_model.entity_at(index))
                    }
                }
            }

            Image {
                id: flux_image_container
                Layout.fillWidth: true
                Layout.preferredHeight: width
                Layout.minimumHeight: 48

                source: map_selector.currentIndex == 0 ?
                            AppData.flux.current_image
                          :
                            AppData.flux.current_point_image
                fillMode: Image.PreserveAspectFit

                mipmap: true
                cache: false

                STIconButton {
                    icon: "\uf019"
                    toolTip: "Save the displayed flux-map image"

                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.margins: 10

                    onClicked: save_image_dialog.open()

                    FileDialog {
                        id: save_image_dialog

                        fileMode: FileDialog.SaveFile
                        defaultSuffix: "png"
                        currentFolder: StandardPaths.standardLocations(
                                           StandardPaths.DocumentsLocation
                                           )[0]

                        onAccepted: {
                            AppData.flux.save_image(flux_image_container.source,
                                                    save_image_dialog.currentFile);
                        }
                    }
                }

                ShadowedRectangle {
                    id: color_bar_container

                    property bool isClosed: false

                    property real openWidth: Math.max(lower_bound.implicitWidth + 16,
                                                      upper_bound.implicitWidth + 16)

                    width: isClosed ? 10 : openWidth
                    height: parent.height / 2
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    anchors.bottomMargin: 6
                    anchors.leftMargin: 6

                    visible: map_selector.currentIndex === 0
                    opacity: legend_hover.containsMouse ? 1 : .5

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                        }
                    }

                    Item {
                        id: color_bar
                        width: 24
                        anchors.fill: parent
                        anchors.margins: 2

                        Image {
                            rotation: -90
                            source: root.flux_module.current_color_map_source
                            anchors.centerIn: parent
                             width: parent.height
                            height: parent.width

                            Label {
                                anchors.centerIn: parent
                                text: "W/m²"
                                style: Label.Outline
                                styleColor: Qt.alpha(Material.backgroundColor, 0.5)

                                opacity: !color_bar_container.isClosed
                            }
                        }

                        Label {
                            id: upper_bound
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.top: parent.top
                            anchors.topMargin: 4
                            text: root.formatStatistic(
                                      root.flux_module.current_flux_stats.peak_flux
                                      )

                            opacity: !color_bar_container.isClosed
                        }

                        Label {
                            id: lower_bound
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 4
                            text: root.formatStatistic(0)

                            opacity: !color_bar_container.isClosed
                        }
                    }



                    MouseArea {
                        id: legend_hover
                        hoverEnabled: true
                        anchors.fill: parent

                        onClicked: color_bar_container.isClosed = !color_bar_container.isClosed
                    }
                }
            }

            STComboBar {
                id: map_selector
                Layout.fillWidth: true
                iconModel: ["\uf00a", "\uf141"]
                model: ["Bins", "Points"]
            }

            STSwitch {
                text: "Hide Scene Geometry"
                checked: !AppData.flux.show_other_geometry
                onToggled: AppData.flux.show_other_geometry = !checked
            }

        }

        STFormPanel {
            collapsible: true
            title: "Map Statistics"
            documentationKey: "analyze.flux.stats"
            visible: root.flux_module.has_results
                     && AppData.flux.current_image.length > 0

            STFormRow {
                label: "Contributing Hits"

                Label {
                    Layout.fillWidth: true
                    text: Number(root.flux_module.current_flux_stats.plotted_ray_count)
                              .toLocaleString(Qt.locale(), "f", 0)
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Plotted Power"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.plotted_power, "W")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Peak Flux"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.peak_flux, "W/m²")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Minimum Flux"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.min_flux, "W/m²")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Average Flux"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.average_flux, "W/m²")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Flux Standard Deviation"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.sigma_flux, "W/m²")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Uniformity"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.uniformity)
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Peak Flux Uncertainty"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.peak_flux_uncertainty, "%")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Average Flux Uncertainty"

                Label {
                    Layout.fillWidth: true
                    text: formatStatistic(root.flux_module.current_flux_stats.average_flux_uncertainty, "%")
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Global Centroid"

                Label {
                    property vector3d cent: root.flux_module.current_flux_stats.centroid
                    Layout.fillWidth: true
                    text: root.formatCentroid(cent)
                    horizontalAlignment: Text.AlignRight
                }
            }

            STFormRow {
                label: "Local Centroid"

                Label {
                    property vector3d cent: root.flux_module.current_flux_stats.local_centroid
                    Layout.fillWidth: true
                    text: root.formatCentroid(cent)
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        STFormPanel {
            collapsible: true
            title: "Compute Flux Map"
            visible: root.flux_module.has_results

            STDocumentationSection {
                key: "analyze.flux.target"
            }

            STFormRow {
                label: "Element"

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    STButton {
                        Layout.fillWidth: true

                        text: AppData.flux.current_entity_name.length ?
                                  AppData.flux.current_entity_name : "Select Element"
                        left_text_icon: "\uf03a"

                        onClicked: entity_pop.open()

                        SelectItemPopup {
                            id: entity_pop
                            source_model: AppData.flux.entity_model

                            onSelectedEntity: (entity) => {
                                AppData.flux.select_entity(entity)
                            }
                        }
                    }

                    STIconButton {
                        icon: "\uf245"
                        toolTip: "Pick element from view"

                        onClicked: {
                            App.view.simulation_content_view = true
                            App.view.mouse_mode = ViewModule.SelectElement
                        }
                    }

                    STIconButton {
                        icon: "\uf140"
                        toolTip: "Orient Camera to Element"
                        enabled: AppData.flux.current_entity.is_valid()

                        onClicked: {
                            App.view.simulation_content_view = true
                            simulation_scene.orient_camera_to_database_position(
                                        AppData.flux.current_entity_position)
                        }
                    }
                }
            }

            STDocumentationSection {
                key: "analyze.flux.contributions"
            }

            STFormRow {
                label: "Ray Contributions"

                STButton {
                    Layout.fillWidth: true
                    text: root.contributionSummary()
                    onClicked: contribution_filter_popup.open()

                    FluxRayFilterPopup {
                        id: contribution_filter_popup
                        filterModel: root.flux_module.pending_flux_maps
                    }
                }
            }

            STFormRow {
                label: "Color Map"

                STColorMapCombo {
                    Layout.fillWidth: true
                    currentValue:
                        AppData.flux.pending_flux_maps.color_map_id
                    onActivated: {
                        AppData.flux.pending_flux_maps.color_map_id =
                                currentValue
                    }
                }
            }

            STDocumentationSection {
                key: "analyze.flux.dni"
            }

            STFormRow {
                label: "Solar DNI (W/m²)"

                STDoubleSpinBox {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 2000.0
                    stepSize: 50.0
                    decimals: 1
                    value: AppData.flux.dni

                    onValueModified: AppData.flux.dni = value
                }
            }

            STDocumentationSection {
                key: "analyze.flux.resolution"
            }

            STSwitch {
                text: "Show Triangle Grid"
                checked: AppData.flux.pending_flux_maps.show_mesh_grid
                onToggled: AppData.flux.pending_flux_maps.show_mesh_grid = checked
            }

            STFormRow {
                label: "Flux Grid Resolution"

                STComboBox {
                    Layout.fillWidth: true

                    model: [
                        { value: 128, text: "128 × 128" },
                        { value: 256, text: "256 × 256" },
                        { value: 512, text: "512 × 512" },
                        { value: 1024, text: "1024 × 1024" },
                        { value: 2048, text: "2048 × 2048" },
                        { value: 4096, text: "4096 × 4096" },
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentValue: root.closestFluxGridResolution(
                                      AppData.flux.pending_flux_maps.image_resolution)

                    onActivated: {
                        AppData.flux.pending_flux_maps.image_resolution =
                                Qt.size(currentValue, currentValue)
                    }

                    Component.onCompleted: {
                        const size =
                                AppData.flux.pending_flux_maps.image_resolution
                        const resolution = root.closestFluxGridResolution(size)
                        if (size.width !== resolution
                                || size.height !== resolution) {
                            AppData.flux.pending_flux_maps.image_resolution =
                                    Qt.size(resolution, resolution)
                        }
                    }
                }
            }

            STFormRow {
                label: "Smoothing"

                STComboBox {
                    Layout.fillWidth: true

                    model: [
                        { value: 0, text: "None" },
                        { value: 2, text: "Light (2-bin radius)" },
                        { value: 4, text: "Medium (4-bin radius)" },
                        { value: 8, text: "Strong (8-bin radius)" },
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentValue:
                        AppData.flux.pending_flux_maps.smoothing_radius

                    onActivated: {
                        AppData.flux.pending_flux_maps.smoothing_radius =
                                currentValue
                    }
                }
            }

            STFormRow {
                label: "Surface Detail"

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 16
                    value: AppData.flux.pending_flux_maps.mesh_resolution_multiply

                    onValueModified: {
                        AppData.flux.pending_flux_maps.mesh_resolution_multiply = value
                    }
                }
            }

            ListView {
                Layout.fillWidth: true

                Layout.preferredHeight: 120

                model: root.flux_module.pending_flux_maps

                visible: count > 0

                delegate: Rectangle {
                    required property var entity;
                    required property int progress;

                    width: ListView.view.width
                    height: 24

                    color: Qt.alpha("black", .25)

                    radius: 10

                    border.color: Material.dividerColor
                    border.width: 1

                    ShadowedRectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        width: progress / 100 * parent.width
                    }

                    Label {
                        anchors.fill: parent

                        text: "Map for " + parent.entity + ": " + parent.progress + "%"

                        verticalAlignment: Qt.AlignVCenter
                        horizontalAlignment: Qt.AlignHCenter
                    }

                }
            }

            STButton {
                Layout.fillWidth: true
                enabled: AppData.flux.current_entity.is_valid()

                text: "Compute Map"
                left_text_icon: "\uf0da"

                onClicked: {
                    AppData.flux.start_generate()
                }
            }

            STButton {
                Layout.fillWidth: true

                text: "Compute Maps for All Elements"
                left_text_icon: "\uf252"

                onClicked: batch_warn.open()

                STDialog {
                    id: batch_warn
                    title: "Batch Compute All Maps"

                    height: 256
                    width: 320

                    ColumnLayout {
                        anchors.fill: parent
                        Label {
                            Layout.fillWidth: true
                            text: "This will generate flux maps for ALL elements which have an interacting ray."
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "Depending on the size of the scene, this operation could take a significant amount of time and compute resources."
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        }
                    }

                    standardButtons: Dialog.Ok | Dialog.Cancel

                    onAccepted: {
                        AppData.flux.start_generate_batch()
                    }
                }

                STPopup {
                    id: batch_progress

                    modal: true

                    closePolicy: STPopup.NoAutoClose

                    parent: Overlay.overlay
                    anchors.centerIn: Overlay.overlay

                    contentWidth: progress_view.implicitWidth
                    contentHeight: progress_view.implicitHeight

                    Connections {
                        target: AppData.flux
                        function onStarted_batch() {
                            batch_progress.open()
                        }
                    }

                    Connections {
                        target: AppData.flux
                        function onBatch_progress(value, max) {
                            if (max <= 0) batch_progress.close()
                            batch_progress_bar.from = 0
                            batch_progress_bar.to = Math.max(max, value)
                            batch_progress_bar.value = value
                        }
                    }

                    ColumnLayout {
                        id: progress_view
                        anchors.fill: parent


                        ProgressBar {
                            Layout.fillWidth: true
                            id: batch_progress_bar
                            from: 0
                            to: 0
                            value: 0
                        }

                        STDangerousButton {
                            id: cancel_batch_button

                            text: "Cancel"

                            onClicked: AppData.flux.cancel_batch()
                        }
                    }
                }
            }

        }

        STFormPanel {
            // Disable for now until this is a working feature
            visible: false

            collapsible: true
            title: "Normalized Ray Volume"
            documentationKey: "analyze.flux.volume"

            STFormRow {
                label: "Grid Resolution"

                STSpinBox {
                    id: resolution_spin
                    Layout.fillWidth: true

                    value: 512
                    from: 64
                    to: 2048
                }
            }

            STButton {
                enabled: !AppData.flux.ray_volume_flux_in_progress
                Layout.fillWidth: true

                text: "Start Raster"
                left_text_icon: "\uf0da"

                onClicked: {
                    AppData.flux.start_generate_volume_flux(resolution_spin.value)
                }
            }

            STPropertySeparator {
                title: "Isosurface"
            }

            STFormRow {
                label: "Isovalue"

                STDoubleSpinBox {
                    id: iso_spin
                    Layout.fillWidth: true

                    value: 0.90
                    from: 0.0
                    stepSize: .01
                    decimals: 2
                    to: 1.0
                }
            }

            STButton {
                enabled: !AppData.flux.ray_volume_flux_in_progress
                Layout.fillWidth: true

                text: "Generate Surface"
                left_text_icon: "\uf0da"

                onClicked: {
                    AppData.flux.start_generate_isosurface(iso_spin.value)
                }
            }

            STSwitch {
                text: "Visible"
                checked: AppData.flux.show_flux_volume
                onToggled: AppData.flux.show_flux_volume = checked
            }

        }
    }
}
