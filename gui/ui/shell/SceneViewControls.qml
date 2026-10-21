import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root
    spacing: 0

    STIconButton {
        icon: AppData.view.sim.camera === SimulationViewState.WASD
              ? "\uf047" : "\ue4bb"
        toolTip: "Camera Mode"

        onClicked: camera_popup.open()

        STPopup {
            id: camera_popup

            width: 250
            x: -width - 16
            y: (parent.height - height) / 2

            ColumnLayout {
                width: parent.width

                InlineDocumentation {
                    key: "view.camera"
                    title: "Cameras"
                }

                Label {
                    text: "Camera Controls"
                }

                STComboBar {
                    currentIndex: AppData.view.sim.camera
                    onCurrentIndexChanged: AppData.view.sim.camera = currentIndex
                    Layout.fillWidth: true
                    model: ["FPS", "Orbit"]
                    iconModel: ["\uf03d", "\uf135"]
                }

                SliderField {
                    value: AppData.view.sim.fps_walk_speed
                    from: 10
                    to: 100
                    text: "FPS Camera Speed"
                    visible: AppData.view.sim.camera === 0

                    onValueChanged: App.view.sim.fps_walk_speed = value
                }

                InlineDocumentation {
                    key: "view.perspective"
                    title: "Camera Perspectives"
                }

                Label {
                    text: "Camera Projection"
                }

                STComboBar {
                    currentIndex: AppData.view.sim.perspective
                    onCurrentIndexChanged: AppData.view.sim.perspective = currentIndex
                    Layout.fillWidth: true
                    model: ["Perspective", "Orthographic"]
                    iconModel: ["\uf1b2", "\uf0c8"]
                }
            }
        }
    }

    STIconButton {
        icon: "\uf279"
        toolTip: "View Tools"

        onClicked: view_tools_popup.open()

        STPopup {
            id: view_tools_popup

            width: 250
            x: -width - 16
            y: (parent.height - height) / 2

            ColumnLayout {
                width: parent.width

                STButton {
                    Layout.fillWidth: true
                    text: "Fit All in View"
                    left_text_icon: "\uf279"
                    onClicked: {
                        simulation_scene.fit_all_in_view()
                        view_tools_popup.close()
                    }
                }
            }
        }
    }

    STIconButton {
        icon: "\uf568"
        toolTip: "Alignment"

        onClicked: camera_alignment_popup.open()

        STPopup {
            id: camera_alignment_popup
            width: 250
            x: -width - 16
            y: (parent.height - height) / 2

            GridLayout {
                columns: 2

                Layout.fillWidth: true

                uniformCellWidths: true
                uniformCellHeights: true

                STButton {
                    text: "Reset"
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    onClicked: simulation_scene.reset_camera_view()
                }

                STButton {
                    text: "45° View"
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    onClicked: simulation_scene.align_to_pretty_view()
                }

                STButton {
                    text: "-X"
                    onClicked: simulation_scene.align_to_axis(
                                   CameraController.Axis.X, true)
                }

                STButton {
                    text: "X"
                    onClicked: simulation_scene.align_to_axis(
                                   CameraController.Axis.X, false)
                }

                STButton {
                    text: "-Y"
                    onClicked: simulation_scene.align_to_axis(
                                   CameraController.Axis.Y, true)
                }

                STButton {
                    text: "Y"
                    onClicked: simulation_scene.align_to_axis(
                                   CameraController.Axis.Y, false)
                }

                STButton {
                    text: "-Z"
                    onClicked: simulation_scene.align_to_axis(
                                   CameraController.Axis.Z, true)
                }

                STButton {
                    text: "Z"
                    onClicked: simulation_scene.align_to_axis(
                                   CameraController.Axis.Z, false)
                }
            }
        }
    }

    STIconButton {
        icon: "\uf625"
        toolTip: "Performance"

        onClicked: performance_popup.open()

        STPopup {
            id: performance_popup
            width: 260
            x: -width - 16
            y: (parent.height - height) / 2

            GridLayout {
                width: parent.width
                columns: 2

                STPropertySeparator {
                    title: "Performance"
                }

                STPropertyLabel {
                    text: "Surface Width"
                }

                STDoubleSpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 10
                    stepSize: 0.01
                    decimals: 2
                    value: App.view.sim.geometry_thickness
                    onValueModified: App.view.sim.geometry_thickness = value
                }

                STPropertyLabel {
                    text: "Subdivision Scale"
                }

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 10
                    value: App.view.sim.geometry_subdivision_scale
                    onValueModified: {
                        App.view.sim.geometry_subdivision_scale = value
                    }
                }

                STSwitch {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: "Show Grid"
                    checked: App.view.sim.show_grid
                    onToggled: App.view.sim.show_grid = checked
                }
            }
        }
    }

}

