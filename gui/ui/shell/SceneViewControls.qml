import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root
    spacing: 0

    STIconButton {
        id: camera_button
        icon: AppData.view.sim.camera === SimulationViewState.WASD
              ? "\uf047" : "\ue4bb"
        toolTip: "Camera Mode"

        onClicked: camera_popup.open()

        SceneViewPopup {
            id: camera_popup
            title: qsTr("Camera & Projection")
            icon: camera_button.icon

            STDocumentationSection {
                key: "view.camera"
            }

            STComboBar {
                currentIndex: AppData.view.sim.camera
                onCurrentIndexChanged: AppData.view.sim.camera = currentIndex
                Layout.fillWidth: true
                model: [qsTr("FPS"), qsTr("Orbit")]
                iconModel: ["\uf03d", "\uf135"]
            }

            SliderField {
                value: AppData.view.sim.fps_walk_speed
                from: 10
                to: 100
                text: qsTr("FPS Camera Speed")
                visible: AppData.view.sim.camera === SimulationViewState.WASD

                onValueChanged: App.view.sim.fps_walk_speed = value
            }

            STDocumentationSection {
                key: "view.perspective"
            }

            STComboBar {
                currentIndex: AppData.view.sim.perspective
                onCurrentIndexChanged: AppData.view.sim.perspective = currentIndex
                Layout.fillWidth: true
                model: [qsTr("Perspective"), qsTr("Orthographic")]
                iconModel: ["\uf1b2", "\uf0c8"]
            }
        }
    }

    STIconButton {
        id: view_tools_button
        icon: "\uf279"
        toolTip: "View Tools"

        onClicked: view_tools_popup.open()

        SceneViewPopup {
            id: view_tools_popup
            title: qsTr("View Tools")
            icon: view_tools_button.icon

            STButton {
                Layout.fillWidth: true
                text: qsTr("Fit All in View")
                left_text_icon: "\uf279"
                onClicked: {
                    simulation_scene.fit_all_in_view()
                    view_tools_popup.close()
                }
            }
        }
    }

    STIconButton {
        id: alignment_button
        icon: "\uf568"
        toolTip: "Alignment"

        onClicked: camera_alignment_popup.open()

        SceneViewPopup {
            id: camera_alignment_popup
            title: qsTr("Alignment")
            icon: alignment_button.icon

            function alignToAxis(axis, reversed) {
                simulation_scene.align_to_axis(axis, reversed)
                close()
            }

            GridLayout {
                columns: 2

                Layout.fillWidth: true

                uniformCellWidths: true
                uniformCellHeights: true

                STButton {
                    text: qsTr("Reset")
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    onClicked: {
                        simulation_scene.reset_camera_view()
                        camera_alignment_popup.close()
                    }
                }

                STButton {
                    text: qsTr("45° View")
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    onClicked: {
                        simulation_scene.align_to_pretty_view()
                        camera_alignment_popup.close()
                    }
                }

                STButton {
                    text: "-X"
                    onClicked: camera_alignment_popup.alignToAxis(
                                   CameraController.Axis.X, true)
                }

                STButton {
                    text: "X"
                    onClicked: camera_alignment_popup.alignToAxis(
                                   CameraController.Axis.X, false)
                }

                STButton {
                    text: "-Y"
                    onClicked: camera_alignment_popup.alignToAxis(
                                   CameraController.Axis.Y, true)
                }

                STButton {
                    text: "Y"
                    onClicked: camera_alignment_popup.alignToAxis(
                                   CameraController.Axis.Y, false)
                }

                STButton {
                    text: "-Z"
                    onClicked: camera_alignment_popup.alignToAxis(
                                   CameraController.Axis.Z, true)
                }

                STButton {
                    text: "Z"
                    onClicked: camera_alignment_popup.alignToAxis(
                                   CameraController.Axis.Z, false)
                }
            }
        }
    }

    STIconButton {
        id: background_button
        icon: "\uf6c4"
        toolTip: "Scene Background"

        onClicked: background_popup.open()

        SceneViewPopup {
            id: background_popup
            title: qsTr("Scene Background")
            icon: background_button.icon

            GridLayout {
                Layout.fillWidth: true
                columns: 2

                STPropertyLabel {
                    text: qsTr("Background")
                }

                STComboBox {
                    Layout.fillWidth: true
                    model: [
                        qsTr("Day"),
                        qsTr("Blueprint"),
                        qsTr("Adaptive"),
                        qsTr("Realistic")
                    ]
                    currentIndex: App.view.sim.sky
                    onActivated: App.view.sim.sky = currentIndex
                }

                STPropertyLabel {
                    visible: App.view.sim.sky === SimulationViewState.Realistic
                    text: qsTr("Environment")
                }

                STComboBox {
                    Layout.fillWidth: true
                    visible: App.view.sim.sky === SimulationViewState.Realistic
                    model: [
                        qsTr("Clear"),
                        qsTr("Partly Cloudy"),
                        qsTr("Low Sun")
                    ]
                    currentIndex: App.view.sim.realistic_sky
                    onActivated: App.view.sim.realistic_sky = currentIndex
                }

                Label {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    visible: App.view.sim.sky === SimulationViewState.Realistic
                    text: qsTr("Realistic skies are decorative and do not affect ray tracing.")
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                }
            }
        }
    }

    STIconButton {
        id: performance_button
        icon: "\uf625"
        toolTip: "Performance"

        onClicked: performance_popup.open()

        SceneViewPopup {
            id: performance_popup
            title: qsTr("Performance")
            icon: performance_button.icon

            GridLayout {
                Layout.fillWidth: true
                columns: 2

                STPropertyLabel {
                    text: qsTr("Surface Width")
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
                    text: qsTr("Subdivision Scale")
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
                    text: qsTr("Show Grid")
                    checked: App.view.sim.show_grid
                    onToggled: App.view.sim.show_grid = checked
                }
            }
        }
    }

}

