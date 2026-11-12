import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

STScrollView {
    id: root

    Layout.fillHeight: true
    Layout.fillWidth: true

    contentWidth: availableWidth

    property bool showProgressStatus: false
    property bool simulationRunning: AppData.simulation.is_running

    onSimulationRunningChanged: {
        if (simulationRunning) {
            hideIdleStatusTimer.stop()
            showProgressStatus = true
        } else if (showProgressStatus) {
            hideIdleStatusTimer.restart()
        }
    }

    Timer {
        id: hideIdleStatusTimer
        interval: 3000
        repeat: false
        onTriggered: root.showProgressStatus = AppData.simulation.is_running
    }

    Component.onCompleted: showProgressStatus = simulationRunning

    ColumnLayout {
        width: root.availableWidth

        STFormPanel {
            title: "Trace Configuration"

            STDocumentationSection {
                key: "simulate.trace"
            }

            STComboBox {
                Layout.fillWidth: true
                model: AppData.simulation.runners
                textRole: "name"
                valueRole: "runner"
                currentIndex: AppData.simulation.runners.index_of(
                                  AppData.simulation.runner)
                onActivated: (index) => {
                    AppData.simulation.runner =
                            AppData.simulation.runners.runner_at(index)
                }
            }

            STDocumentationSection {
                key: "simulate.trace.rays"
            }

            STFormRow {
                label: "# of Rays"

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 1000000000
                    stepSize: 1
                    editable: true

                    property bool keyStepUp: false
                    property bool keyStepDown: false
                    Keys.onPressed: (event) => {
                        keyStepUp = event.key === Qt.Key_Up
                        keyStepDown = event.key === Qt.Key_Down
                    }

                    // Typed values are used as-is; only button/key steps are rescaled
                    onValueModified: {
                        var current = AppData.simulation.ray_count
                        var newValue = value
                        if (keyStepDown || value < current) {
                            // Hack: I couldn't get up/down.pressed to work because of indicator
                            if (current - value === stepSize)
                                newValue = Math.max(from, Math.round(current / 10))
                        }
                        if (keyStepUp || value > current) {
                            if (value - current === stepSize)
                                newValue = Math.min(to, current * 10)
                        }
                        AppData.simulation.update_ray_count(newValue)
                        keyStepUp = keyStepDown = false
                    }
                    value: AppData.simulation.ray_count
                }
            }

            STFormRow {
                label: "Max # Rays Traced"

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1

                    to: 1000000000

                    property bool keyStepUp: false
                    property bool keyStepDown: false
                    Keys.onPressed: (event) => {
                        keyStepUp = event.key === Qt.Key_Up
                        keyStepDown = event.key === Qt.Key_Down
                    }

                    onValueModified: {
                        var current = AppData.simulation.max_ray_count
                        var newValue = value
                        if (keyStepDown || value < current) {
                            // Hack: I couldn't get up/down.pressed to work because of indicator
                            if (current - value === stepSize)
                                newValue = Math.max(from, Math.round(current / 10))
                        }
                        if (keyStepUp || value > current) {
                            if (value - current === stepSize)
                                newValue = Math.min(to, current * 10)
                        }
                        AppData.simulation.update_max_ray_count(newValue)
                        keyStepUp = keyStepDown = false
                    }
                    value: AppData.simulation.max_ray_count
                }
            }

            STFormRow {
                label: "# of CPU Cores"
                visible: AppData.simulation.runner < 2

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    value: AppData.simulation.max_threads
                    to: 64
                    onValueModified: AppData.simulation.max_threads = value
                }
            }

            STFormRow {
                label: "Seed Value"

                STSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    value: AppData.simulation.seed_value
                    to: 10000000
                    onValueModified: AppData.simulation.seed_value = value
                }
            }

            STDocumentationSection {
                key: "simulate.trace.options"
            }

            STFormRow {
                label: "Options"

                ColumnLayout {
                    Layout.fillWidth: true

                    STSwitch {
                        text: "Sun Shape"
                        checked: AppData.simulation.sun_shape
                        onToggled: AppData.simulation.sun_shape = checked
                    }

                    STSwitch {
                        text: "Optical Errors"
                        checked: AppData.simulation.optical_errors
                        onToggled: AppData.simulation.optical_errors = checked
                    }

                    STSwitch {
                        text: "Tracer Debug (if applicable)"
                        //checked: AppData.simulation.optical_errors
                        //onToggled: AppData.simulation.optical_errors = checked
                    }
                }
            }
        }

        STFormPanel {
            collapsible: true
            title: "Execution"
            documentationKey: "simulate.trace.results"

            STFormRow {
                label: "Progress"
                visible: root.showProgressStatus

                ProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: AppData.simulation.progress

                    enabled: AppData.simulation.is_running
                }
            }

            STFormRow {
                label: "Stage"
                visible: root.showProgressStatus

                Label {
                    Layout.fillWidth: true
                    text: AppData.simulation.is_running ?
                              AppData.simulation.current_stage : "Idle"
                }
            }

            STButton {
                Layout.fillWidth: true
                text: "Start Trace"
                left_text_icon: "\uf0da"
                visible: !AppData.simulation.is_running
                onClicked: {
                    AppData.simulation.run()
                }
            }

            STDangerousButton {
                Layout.fillWidth: true
                text: "Cancel Trace"
                left_text_icon: "\uf04d"
                visible: AppData.simulation.is_running
                onClicked: {
                    AppData.simulation.cancel()
                }
            }

        }
    }
}
