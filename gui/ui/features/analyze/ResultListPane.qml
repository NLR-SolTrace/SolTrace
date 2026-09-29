import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root

    signal closeRequested()

    spacing: 8

    function formatRayCount(count) {
        return Number(count).toLocaleString(Qt.locale(), "f", 0) + " rays"
    }

    function showResultMode() {
        App.view.simulation_content_view = true
        App.view.workflow_phase = ViewModule.Analyze
    }

    function showSceneMode() {
        App.view.simulation_content_view = false
    }

    InlineDocumentation {
        key: "analyze.results"
        Layout.fillWidth: true
    }

    Label {
        Layout.fillWidth: true
        text: "Simulation Results"
        font.bold: true
    }

    Label {
        Layout.fillWidth: true
        visible: result_list.count === 0
        text: "No simulation results yet."
        opacity: 0.7
        horizontalAlignment: Text.AlignHCenter
    }

    ListView {
        id: result_list
        property int previousCount: 0

        Layout.fillHeight: true
        Layout.fillWidth: true
        clip: true
        model: AppData.simulation.results

        ScrollBar.vertical: STScrollBar { }

        onCountChanged: {
            if (count > previousCount) {
                currentIndex = count - 1
            } else if (count === 0) {
                currentIndex = -1
            } else if (currentIndex < 0 || currentIndex >= count) {
                currentIndex = count - 1
            }
            previousCount = count
        }

        delegate: STExpandingItemDelegate {
            id: resultDelegate
            required property int index

            required property string name
            required property var when
            required property var ray_count

            text: name
            highlighted: result_list.currentIndex === index

            onDeleteRequested: AppData.simulation.delete_result(index)

            onRename: function(text) {
                AppData.simulation.rename_result(index, text)
            }

            onClicked: {
                result_list.currentIndex = index
                AppData.simulation.select_result(index)
                root.showResultMode()
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: Qt.formatDateTime(resultDelegate.when,
                                            "yyyy-MM-dd hh:mm:ss")
                    opacity: 0.7
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: root.formatRayCount(resultDelegate.ray_count)
                    opacity: 0.85
                    elide: Text.ElideRight
                }
            }

            STIconButton {
                icon: "\uf24d"
                label: "Create Scene"
                toolTip: "Create Scene from Result"

                onClicked: {
                    AppData.simulation.select_result(index)
                    AppData.simulation.duplicate_current_result_for_edit()
                    root.showSceneMode()
                    root.closeRequested()
                }
            }

            STIconButton {
                icon: "\uf019"
                label: "Export"
                toolTip: "Export Result"

                onClicked: {
                    AppData.simulation.select_result(index)
                    AppData.exporter.export_current()
                }
            }

            STIconButton {
                icon: "\uf2ed"
                label: "Delete"
                toolTip: "Delete Result"
                onClicked: confirmDelete.open()

                STDialog {
                    id: confirmDelete
                    title: "Confirm Deletion"

                    ColumnLayout {
                        Label {
                            Layout.fillWidth: true
                            text: "Are you sure you wish to delete the following simulation result?"
                            wrapMode: Text.Wrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: resultDelegate.name
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "This cannot be undone."
                            opacity: 0.7
                        }
                    }

                    standardButtons: Dialog.Ok | Dialog.Cancel
                    onAccepted: resultDelegate.deleteRequested()
                }
            }
        }
    }
}
