import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STScrollView {
    id: root

    Layout.fillWidth: true
    Layout.fillHeight: true
    contentWidth: availableWidth
    clip: true

    ColumnLayout {
        width: root.availableWidth
        spacing: 12

        Header {
            text: "Settings"
        }

        STPropertyPanel {
            Layout.fillWidth: true

            title: "Recording"
            collapsed: false

            ColumnLayout {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: "Recording mode hides prerelease warnings from the interface. It does not change simulation behavior."
                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                }

                STSwitch {
                    text: "Recording mode"
                    checked: AppData.recording_mode
                    onToggled: AppData.recording_mode = checked
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
