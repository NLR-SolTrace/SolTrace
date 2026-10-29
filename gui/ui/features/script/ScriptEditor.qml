import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs

import SolTrace

STPopup {
    id: root
    parent: Overlay.overlay

    property var module: AppData.script
    property string sourceLabel: qsTr("Unsaved script")
    property string workingCode: ""
    property bool dirty: workingCode !== module.code

    signal changesApplied

    modal: true
    focus: true
    enable_shadow: true
    closePolicy: Popup.CloseOnEscape

    width: Math.max(420, Math.min(parent.width - 48, 1100))
    height: Math.max(360, Math.min(parent.height - 48, 820))
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)

    onOpened: {
        workingCode = module.code
        text_edit_area.forceActiveFocus()
    }

    function discardAndClose() {
        workingCode = module.code
        close()
    }

    function applyChanges() {
        module.code = workingCode
        changesApplied()
    }

    contentItem: ColumnLayout {
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Script Editor")
                    font.bold: true
                    elide: Label.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: root.sourceLabel
                    opacity: 0.65
                    elide: Label.ElideMiddle
                }
            }

            STIconButton {
                icon: "\uf00d"
                toolTip: root.dirty ? qsTr("Discard Changes") : qsTr("Close")
                onClicked: root.discardAndClose()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: Qt.rgba(0, 0, 0, 0.18)
            border.width: 1
            border.color: text_edit_area.activeFocus
                          ? Material.accentColor : Material.dividerColor

            STScrollView {
                id: editor_scroll
                anchors.fill: parent
                anchors.margins: 1
                clip: true
                contentWidth: Math.max(availableWidth,
                                       text_edit_area.contentWidth)

                TextArea {
                    id: text_edit_area

                    width: Math.max(editor_scroll.availableWidth, contentWidth)
                    text: root.workingCode
                    selectByMouse: true
                    wrapMode: TextEdit.NoWrap
                    // We'll test this out see if it works nicely.
                    font.family: Qt.platform.os === "osx" ? "Menlo"
                                                         : Qt.platform.os === "windows"
                                                           ? "Consolas"
                                                           : "DejaVu Sans Mono"
                    tabStopDistance: font.pixelSize * 4
                    background: Item { }

                    onTextEdited: root.workingCode = text
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: error_column.implicitHeight + 12
            radius: 8
            color: Qt.rgba(0.9, 0.25, 0.25, 0.10)
            visible: root.module.parse_errors.length > 0 && !root.dirty

            ColumnLayout {
                id: error_column
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 6
                spacing: 2

                Label {
                    text: qsTr("Script errors")
                    font.bold: true
                    color: Material.color(Material.Red)
                }

                Label {
                    Layout.fillWidth: true
                    text: root.module.parse_errors.join("\n")
                    color: Material.color(Material.Red)
                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.dirty
            text: qsTr("Apply the changes to validate the script.")
            opacity: 0.65
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }

        RowLayout {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            STButton {
                text: qsTr("Save As...")
                onClicked: save_script_pop.open()

                FileDialog {
                    id: save_script_pop

                    fileMode: FileDialog.SaveFile
                    currentFile: StandardPaths.standardLocations(
                                     StandardPaths.DocumentsLocation
                                     )[0]
                    nameFilters: ["JavaScript files (*.js)"]
                    defaultSuffix: "js"
                    onAccepted: {
                        var ret = root.module.save_to(selectedFile,
                                                    text_edit_area.text)

                        if (ret) return;

                        save_failed_pop.open()
                    }
                }

                STDialog {
                    id: save_failed_pop

                    modal: true

                    title: qsTr("Script save failed")

                    Label {
                        text: qsTr("Save failed, check permissions and try again.")
                    }

                    standardButtons: STDialog.Ok
                }
            }

            STButton {
                text: root.dirty ? qsTr("Discard") : qsTr("Close")
                onClicked: root.discardAndClose()
            }

            STButton {
                text: qsTr("Apply")
                enabled: root.dirty
                onClicked: root.applyChanges()
            }
        }
    }
}
