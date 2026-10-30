import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

DropArea {
    id: root

    anchors.fill: parent

    Rectangle {
        id: overlay_rect
        visible: root.containsDrag

        anchors.fill: parent
        color: Qt.alpha(Material.backgroundColor, .5)

        Label {
            anchors.centerIn: parent

            width: parent.width / 2
            height: parent.height / 2

            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter

            font.pointSize: 22

            text: qsTr("Drop file here to load")
        }
    }

    onEntered: function(drag) {
        if (!App.file_source.is_loading
                && drag.hasUrls
                && drag.urls.length > 0
                && App.file_source.validate_file(drag.urls[0])) {
            drag.accept(Qt.CopyAction)
        } else {
            drag.accepted = false
        }
    }

    onDropped: function(drop) {
        if (App.file_source.is_loading
                || !drop.hasUrls
                || drop.urls.length === 0)
            return

        // Only handle the first dropped file for the moment.
        var to_load = drop.urls[0]
        if (!App.file_source.validate_file(to_load))
            return

        console.log("Handling dropped file: ", to_load)

        App.file_source.add_recent_file(to_load)
        App.file_source.load_url(to_load)

        drop.accept(Qt.CopyAction)
    }
}
