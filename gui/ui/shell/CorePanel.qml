import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

Item {
    id: root
    property var blur_source

    onWidthChanged: {
        App.view.fit_panels(width, false, true)
        //bottom_bar.collapsed = root.width < bottom_bar.normal_width
    }

    // C++ -> QML (consume width from ViewModule)
    Binding {
        target: left_panel
        property: "SplitView.preferredWidth"
        value: App.view.left_panel.width
        when: left_panel.visible
    }

    Binding {
        target: right_panel
        property: "SplitView.preferredWidth"
        value: App.view.right_panel.width
        when: right_panel.visible
    }

    // QML -> C++ (write back drag-resized widths, then let fit_panels snap)
    Connections {
        target: left_panel
        function onWidthChanged() {
            if (!split.resizing) return
            if (left_panel.width === App.view.left_panel.width) return
            App.view.left_panel.width = left_panel.width
        }
    }

    Connections {
        target: right_panel
        function onWidthChanged() {
            if (!split.resizing) return
            if (right_panel.width === App.view.right_panel.width) return
            App.view.right_panel.width = right_panel.width
        }
    }

    SplitView {
        id: split
        anchors.fill: parent

        orientation: Qt.Horizontal

        handle: Rectangle {
            implicitWidth: 1
            color: SplitHandle.pressed ? "#80ffffff"
                                       : "#20ffffff"
        }

        LeftPanel {
            id: left_panel

            blur_source: root.blur_source
            available_width: root.width

            SplitView.minimumWidth: 400
            SplitView.maximumWidth: 750
            SplitView.fillHeight: true

            visible: App.view.left_panel.visible
            enabled: visible
            opacity: enabled

            radius: 0
            show_border: false
            show_shadow: false
        }

        Item {
            id: center_space
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            SplitView.minimumWidth: App.view.left_panel.visible && App.view.right_panel.visible ? 10 : 0

            TopBar {
                id: top_bar

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 10

                height: left_panel.header_height
                blur_source: root.blur_source
                available_width: root.width
            }

            ShadowedGlassRectangle {
                id: scene_view_toolbar

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10

                width: scene_view_controls.implicitWidth + 8
                height: scene_view_controls.implicitHeight + 8
                radius: width / 2

                blur_source: root.blur_source
                glassColor: App.theme.glassColor

                SceneViewControls {
                    id: scene_view_controls
                    anchors.centerIn: parent
                }
            }
        }

        RightPanel {
            id: right_panel

            blur_source: root.blur_source
            available_width: root.width

            SplitView.minimumWidth: 250
            SplitView.maximumWidth: Math.max(SplitView.minimumWidth,
                                             split.width)
            SplitView.fillHeight: true

            visible: App.view.right_panel.visible
            enabled: visible
            opacity: enabled

            radius: 0
            show_border: false
            show_shadow: false
        }
    }

    // BottomBar {
    //     id: bottom_bar
    //     available_width: root.width
    //     blur_source: root.blur_source
    // }
}
