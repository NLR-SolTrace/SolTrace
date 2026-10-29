import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs

import SolTrace

STScrollView {
    Layout.fillHeight: true
    Layout.fillWidth: true

    id: root

    property var module: AppData.script
    property string sourceLabel: qsTr("No source")
    property bool sourceModified: false
    readonly property string displaySourceLabel: sourceModified
                                                 ? sourceLabel + qsTr(" · Modified")
                                                 : sourceLabel

    readonly property string starterScript: `/*{
  "schema_version": 1,
  "title": "New Script",
  "description": "Describe what this script creates or changes.",
  "parameters": []
}*/

console.log("Script complete.")
`

    contentWidth: availableWidth

    function friendlyName(path) {
        const fileName = String(path).split("/").pop().replace(/\.js$/i, "")
        return fileName.replace(/[_-]+/g, " ")
                       .replace(/\b\w/g, letter => letter.toUpperCase())
    }

    function displayPath(url) {
        const value = decodeURIComponent(String(url))
        return value.split("/").pop() || value
    }

    function load_script(url, label) {
        const requestUrl = String(url).startsWith(":/")
                         ? "qrc" + String(url)
                         : url

        const request = new XMLHttpRequest()
        request.open("GET", requestUrl)
        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                if (request.status === 0 || request.status === 200) {
                    root.module.code = request.responseText
                    root.sourceLabel = label || root.displayPath(url)
                    root.sourceModified = false
                } else {
                    console.warn("Unable to load script", requestUrl, request.status)
                    root.module.notify_error("Could not load the script.")
                }
            }
        }
        request.send()
    }

    function newScript() {
        root.sourceLabel = qsTr("Unsaved script")
        root.sourceModified = false
        root.module.code = root.starterScript
        script_editor.open()
    }

    function outputText() {
        const lines = []
        for (let index = 0; index < output_model.count; ++index)
            lines.push(output_model.get(index).message)
        return lines.join("\n")
    }

    function copyOutput() {
        AppData.copy_text_to_clipboard(outputText())
    }

    function pathToFileUrl(path) {
        return "file://" + path
    }

    ListModel {
        id: output_model
    }

    Connections {
        target: AppData.script

        function onLogged(type, msg) {
            output_model.append({
                type: type,
                message: msg,
            })
        }
    }

    STPopup {
        id: api_popup
        parent: Overlay.overlay
        modal: true
        focus: true
        enable_shadow: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property string apiText: ""
        readonly property real popupMargin: 24

        onOpened: apiText = root.module.api_markdown()

        width: Math.max(320, Math.min(parent.width - popupMargin * 2, 920))
        height: Math.max(320, Math.min(parent.height - popupMargin * 2, 720))
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        contentItem: ColumnLayout {
            spacing: 8

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    text: "Script API"
                    font.bold: true
                    elide: Label.ElideRight
                }

                STIconButton {
                    icon: "\uf00d"
                    toolTip: "Close"
                    onClicked: api_popup.close()
                }
            }

            STScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true

                TextArea {
                    text: api_popup.apiText
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.Wrap
                    textFormat: TextEdit.MarkdownText

                    background: Item {}
                }
            }
        }
    }

    ColumnLayout {
        width: root.availableWidth

        InlineDocumentation {
            key: "script.interface"
            Layout.fillWidth: true
        }

        STPropertyPanel {
            Layout.fillWidth: true

            title: "Source"
            collapsible: true
            columns: 1

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: root.module.title.length
                          ? root.module.title : qsTr("No script loaded")
                    font.bold: true
                    elide: Label.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: root.displaySourceLabel
                    opacity: 0.65
                    elide: Label.ElideMiddle
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                STButton {
                    Layout.fillWidth: true

                    text: "New"
                    onClicked: root.newScript()
                }

                STButton {
                    Layout.fillWidth: true

                    text: "Load"

                    onClicked: file_menu.open()

                    STMenu {
                        id: file_menu
                        MenuItem {
                            text: "Open"
                            onClicked: openFileDialog.open()
                        }

                        STMenu {
                            id: recents_menu
                            title: qsTr("Builtins")
                            enabled: recent_instantiator.count > 0

                            Instantiator {
                                id: recent_instantiator
                                model: root.module.builtin_scripts

                                delegate: MenuItem {
                                    implicitWidth: 140
                                    text: root.friendlyName(modelData)
                                    onTriggered: root.load_script(modelData, text)
                                }
                                onObjectAdded: (index, object) => recents_menu.insertItem(index, object)
                                onObjectRemoved: (index, object) => recents_menu.removeItem(object)
                            }
                        }
                    }

                    FileDialog {
                        id: openFileDialog
                        onAccepted: {
                            var str_file = String(selectedFile)

                            currentFolder = str_file.substring(0, str_file.lastIndexOf("/"))
                            root.load_script(str_file, root.displayPath(str_file))
                        }
                    }
                }

                STButton {
                    Layout.fillWidth: true

                    text: "Edit"

                    onClicked: script_editor.open()

                    ScriptEditor {
                        id: script_editor
                        sourceLabel: root.displaySourceLabel
                        onChangesApplied: root.sourceModified = true
                    }
                }

                STIconButton {
                    icon: "\uf02d"
                    toolTip: qsTr("Show Script API")
                    onClicked: api_popup.open()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: validation_content.implicitHeight + 12
                radius: 8
                color: root.module.valid
                       ? Qt.rgba(0.25, 0.75, 0.4, 0.10)
                       : Qt.rgba(0.9, 0.25, 0.25, 0.10)
                visible: root.module.code.length > 0

                RowLayout {
                    id: validation_content
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 6
                    spacing: 8

                    Label {
                        text: root.module.valid ? "\uf058" : "\uf071"
                        font.family: "Font Awesome 7 Free"
                        color: root.module.valid
                               ? Material.color(Material.Green)
                               : Material.color(Material.Red)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.module.valid
                              ? qsTr("Ready to run")
                              : root.module.parse_errors.join("\n")
                        color: root.module.valid
                               ? Material.foreground
                               : Material.color(Material.Red)
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                    }

                    STIconButton {
                        visible: !root.module.valid
                        icon: "\uf304"
                        toolTip: qsTr("Edit Script")
                        onClicked: script_editor.open()
                    }
                }
            }
        }

        STPropertyPanel {
            Layout.fillWidth: true

            title: "Parameters"
            collapsible: true
            columns: 1

            Label {
                visible: root.module.description.length
                Layout.fillWidth: true

                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                text: root.module.description
            }

            Label {
                visible: parameter_list.count === 0
                Layout.fillWidth: true
                text: root.module.valid
                      ? qsTr("This script has no parameters.")
                      : qsTr("Valid script parameters will appear here.")
                opacity: 0.65
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            }

            ListView {
                id: parameter_list

                visible: count > 0
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentHeight, 6 * 76)

                model: root.module.properties
                clip: true
                spacing: 4

                ScrollBar.vertical: STScrollBar { }

                delegate: ScriptParameterDelegate {
                    width: Math.max(0, ListView.view.width - 8)
                }
            }
        }

        STPropertyPanel {
            Layout.fillWidth: true
            title: "Execute"
            columns: 1

            STFormRow {
                Layout.fillWidth: true
                label: qsTr("Working directory")

                RowLayout {
                    Layout.fillWidth: true

                    STTextField {
                        Layout.fillWidth: true
                        text: root.module.working_directory

                        onAccepted: root.module.working_directory = text
                        onTextEdited: root.module.working_directory = text
                    }

                    STIconButton {
                        icon: "\uf07c"
                        toolTip: qsTr("Select Working Directory")
                        onClicked: workingDirectoryDialog.open()

                        FolderDialog {
                            id: workingDirectoryDialog
                            currentFolder: root.pathToFileUrl(
                                               root.module.working_directory)

                            onAccepted: {
                                const value = String(selectedFolder)
                                root.module.working_directory = decodeURIComponent(
                                            value.replace(/^file:\/\//, ""))
                            }
                        }
                    }
                }
            }

            STButton {
                text: qsTr("Run Script")
                left_text_icon: "\uf04b"
                Layout.fillWidth: true

                enabled: root.module.valid

                onClicked: {
                    output_model.clear()
                    root.module.run()
                }
            }

        }


        STPropertyPanel {
            Layout.fillWidth: true

            title: "Output"
            collapsible: true
            columns: 1

            RowLayout {
                Layout.fillWidth: true
                visible: output_model.count > 0

                Item { Layout.fillWidth: true }

                STIconButton {
                    icon: "\uf0c5"
                    toolTip: qsTr("Copy All Output")
                    onClicked: root.copyOutput()
                }

                STIconButton {
                    icon: "\uf2ed"
                    toolTip: qsTr("Clear Output")
                    onClicked: output_model.clear()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 160
                radius: 8
                color: Qt.rgba(0, 0, 0, 0.12)
                border.width: 1
                border.color: Material.dividerColor

                ListView {
                    id: output_list
                    anchors.fill: parent
                    anchors.margins: 8

                    model: output_model
                    clip: true
                    spacing: 4

                    ScrollBar.vertical: STScrollBar { }

                    delegate: RowLayout {
                        id: output_row

                        required property string message
                        required property int type

                        width: ListView.view.width
                        spacing: 6

                        Label {
                            Layout.alignment: Qt.AlignTop
                            text: output_row.type === 2 ? "\uf057"
                                                       : output_row.type === 1
                                                         ? "\uf071" : "\uf05a"
                            font.family: "Font Awesome 7 Free"
                            color: output_row.type === 2
                                   ? Material.color(Material.Red)
                                   : output_row.type === 1
                                     ? Material.color(Material.Yellow)
                                     : Material.foreground
                        }

                        Label {
                            Layout.fillWidth: true
                            text: output_row.message
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere

                            color: output_row.type === 2
                                   ? Material.color(Material.Red)
                                   : output_row.type === 1
                                     ? Material.color(Material.Yellow)
                                     : Material.foreground
                        }
                    }

                    onCountChanged: positionViewAtEnd()

                    Label {
                        anchors.centerIn: parent
                        width: parent.width - 24
                        visible: output_model.count === 0
                        text: qsTr("Script output will appear here.")
                        opacity: 0.55
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                    }
                }
            }
        }

    }
}
