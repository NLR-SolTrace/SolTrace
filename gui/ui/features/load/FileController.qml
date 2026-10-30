import QtCore
import QtQuick
import QtQuick.Dialogs

import SolTrace

Item {
    id: root

    visible: false
    width: 0
    height: 0

    readonly property var recent_files: App.file_source.recent_files

    function load_new() {
        App.file_source.load_new()
    }

    function open_file() {
        if (App.file_source.open_file_dialog()) {
            return
        }

        if (file_settings.last_selected_file.toString() !== "")
            openFileDialog.selectedFile = file_settings.last_selected_file
        openFileDialog.open()
    }

    function save_current() {
        if (!App.file_source.current_database || App.file_source.is_loading) {
            return
        }

        if (!App.file_source.save_current_dialog()) {
            saveFileDialog.open()
        }
    }

    function load_recent(file_path) {
        var file_url = file_url_text(file_path)
        add_files(file_url)
        App.file_source.load_url(Qt.url(file_url))
    }

    function file_url_text(file_path) {
        return String(file_path)
    }

    function file_name(file_path) {
        var file_url = file_url_text(file_path)
        return decodeURIComponent(file_url.split('/').pop())
    }

    function open_example() {
        exampleFileDialog.open()
    }

    function clear_recent_files() {
        App.file_source.clear_recent_files()
    }

    function add_files(file_path) {
        App.file_source.add_recent_file(Qt.url(file_url_text(file_path)))
    }

    QtObject {
        id: file_settings

        property url last_selected_file
        property url last_selected_folder: StandardPaths.standardLocations(
                                               StandardPaths.DocumentsLocation)[0]
    }

    Settings {
        id: file_settings_storage

        category: "file_history"

        property alias last_selected_file: file_settings.last_selected_file
        property alias last_selected_folder: file_settings.last_selected_folder
    }

    FileDialog {
        id: openFileDialog

        currentFolder: file_settings.last_selected_folder

        onAccepted: {
            var str_file = String(selectedFile)

            file_settings.last_selected_file = selectedFile
            file_settings.last_selected_folder = str_file.substring(
                        0, str_file.lastIndexOf("/"))
            root.add_files(selectedFile.toString())
            App.file_source.load_url(selectedFile)
        }
    }

    FileDialog {
        id: saveFileDialog

        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: ["JSON files (*.json)"]

        onAccepted: App.file_source.save_current(saveFileDialog.selectedFile)

        Settings {
            id: save_file_history

            category: "save_file_history"

            property alias last_selected_file: saveFileDialog.selectedFile
            property alias last_selected_folder: saveFileDialog.currentFolder
        }
    }


    FileDialog {
        id: exampleFileDialog

        currentFolder: App.file_source.examples_folder

        onAccepted: {
            var str_file = String(selectedFile)

            file_settings.last_selected_file = selectedFile
            file_settings.last_selected_folder = str_file.substring(
                        0, str_file.lastIndexOf("/"))
            root.add_files(selectedFile.toString())
            App.file_source.load_url(selectedFile)
        }
    }
}
