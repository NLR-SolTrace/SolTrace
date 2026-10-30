import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects

import SolTrace

Item {
    id: root

    readonly property real expandedImplicitWidth:
        mode_row.implicitWidth + mode_row.anchors.leftMargin
        + mode_row.anchors.rightMargin
    readonly property bool compact: width + 1 < expandedImplicitWidth

    implicitWidth: expandedImplicitWidth
    implicitHeight: mode_row.implicitHeight

    required property var blur_source
    property bool show_header: true
    property int last_db_count: AppData.file_source.rowCount()
    property bool highlighted: false
    function flash_added_data() {
        flash_highlight_animation.restart()
    }

    signal openClicked()

    function selectWorkflow(index) {
        App.view.simulation_content_view = index === 3
        App.view.workflow_phase = index + 1
    }

    component WorkflowItem: Rectangle {
        id: workflow_item

        required property int step
        required property string label
        required property bool active
        property bool has_menu: false

        signal activated()
        signal menuRequested()

        implicitWidth: workflow_item_content.implicitWidth + 16
        implicitHeight: 32
        width: implicitWidth
        height: implicitHeight
        anchors.verticalCenter: parent.verticalCenter
        radius: 12

        color: workflow_item_mouse.containsMouse
               ? Material.rippleColor
               : App.theme.glassColor
        border.width: 1
        border.color: Material.dividerColor

        opacity: active ? 1 : .75


        Behavior on color {
            ColorAnimation { duration: 100 }
        }

        Behavior on opacity {
            NumberAnimation { duration: 100 }
        }

        Behavior on width {
            NumberAnimation {
                duration: 350
                easing.type: Easing.OutElastic
                easing.amplitude: 0.4
                easing.period: 0.3
            }
        }

        RectangularShadow {
            anchors.fill: parent
            radius: parent.radius
            blur: 30
            spread: 2
            color: Material.dropShadowColor
            visible: workflow_item.active
        }

        MouseArea {
            id: workflow_item_mouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: workflow_item.activated()
        }

        Row {
            id: workflow_item_content
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 8

            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: workflow_item.step
                font.pointSize: 16
                font.bold: workflow_item.active
                opacity: workflow_item.active ? 1.0 : 0.5
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 1
                height: 16
                color: Material.dividerColor
            }

            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: workflow_item.label
                font.pointSize: 16
                opacity: workflow_item.active ? 1.0 : 0.5
            }

            STClickableLabel {
                anchors.verticalCenter: parent.verticalCenter
                visible: workflow_item.has_menu
                text: "\uf0d7"
                font.family: "Font Awesome 7 Free"
                opacity: workflow_item.active ? 1.0 : 0.5
                onClicked: workflow_item.menuRequested()
            }
        }
    }

    // Rectangle {
    //     anchors.fill: parent
    //     radius: height / 2
    //     color: App.theme.glassColor
    //     border.width: 1
    //     border.color: Qt.alpha("black", 0.25)
    // }

    Row {
        id: mode_row

        visible: !root.compact

        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        spacing: 4

        Connections {
            target: AppData.file_source

            function onRowsInserted(parent, first, last) {
                root.last_db_count = AppData.file_source.rowCount()
                if (last >= first) {
                    root.flash_added_data()
                }
            }

            function onRowsRemoved(parent, first, last) {
                root.last_db_count = AppData.file_source.rowCount()
                flash_highlight_animation.stop()
                root.highlighted = false
            }
        }

        SequentialAnimation {
            id: flash_highlight_animation

            loops: 3

            ScriptAction {
                script: root.highlighted = true
            }

            PauseAnimation {
                duration: 400
            }

            ScriptAction {
                script: root.highlighted = false
            }

            PauseAnimation {
                duration: 400
            }
        }

        Label {
            anchors.verticalCenter: parent.verticalCenter
            text: "Workflow"
            visible: root.show_header

            //font.family: "CMU Serif"

            font.bold: true

            opacity: 0.65

            rightPadding: 12

            Rectangle {
                id: wf_highlight

                bottomLeftRadius: 42 / 2
                topLeftRadius: 42 / 2
                anchors.fill: parent
                anchors.leftMargin: -20
                anchors.rightMargin: -8
                anchors.topMargin: -12
                anchors.bottomMargin: -12

                color: Material.rippleColor

                opacity: wf_mouse.containsMouse

                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
                }
            }

            MouseArea {
                id: wf_mouse
                anchors.fill: parent
                anchors.margins: -5

                hoverEnabled: true

                onClicked: root.openClicked()
            }
        }

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            color: Material.dividerColor
            visible: root.show_header

            width: 1
            height: 32
        }

        Item {
            visible: root.show_header
            width: visible ? 4 : 0
            height: 1
        }

        WorkflowItem {
            id: load_item

            step: 1
            label: active ? "Load Scene" : "Load"
            active: App.view.workflow_phase === ViewModule.Load
            has_menu: active

            onActivated: {
                App.view.simulation_content_view = false
                App.view.workflow_phase = ViewModule.Load
            }
            onMenuRequested: file_menu.open()

            WorkflowFileMenu {
                id: file_menu
            }
        }

        Label {
            id: file_separator

            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 4
            rightPadding: 8

            font.family: "Font Awesome 7 Free"
            text: "\uf178"

            property bool highlight: load_item.active || configure_item.active

            opacity: highlight ? 1.0 : .50
        }

        WorkflowItem {
            id: configure_item

            step: 2
            label: active ? "Configure Scene" : "Configure"
            active: App.view.workflow_phase === ViewModule.Configure
            has_menu: active

            onActivated: {
                if (App.view.workflow_phase === ViewModule.Configure) {
                    data_pop.open()
                } else {
                    App.view.simulation_content_view = false
                    App.view.workflow_phase = ViewModule.Configure
                }
            }
            onMenuRequested: data_pop.open()
        }

        Label {
            id: configure_separator

            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 4
            rightPadding: 8

            font.family: "Font Awesome 7 Free"
            text: "\uf178"

            property bool highlight: configure_item.active || trace_item.active

            opacity: highlight ? 1.0 : .50

        }

        WorkflowItem {
            id: trace_item

            step: 3
            label: active ? "Trace Scene" : "Trace"
            active: App.view.workflow_phase === ViewModule.Simulate
            has_menu: active

            onActivated: {
                App.view.simulation_content_view = false
                App.view.workflow_phase = ViewModule.Simulate
            }
            onMenuRequested: data_pop.open()
        }

        Label {
            id: analyze_separator

            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 4
            rightPadding: 8

            font.family: "Font Awesome 7 Free"
            text: "\uf178"

            property bool highlight: analyze_item.active || trace_item.active

            opacity: highlight ? 1.0 : .50
        }

        WorkflowItem {
            id: analyze_item

            step: 4
            label: active ? "Analyze Results" : "Analyze"
            active: App.view.workflow_phase === ViewModule.Analyze
            has_menu: active

            onActivated: {
                if (App.view.workflow_phase === ViewModule.Analyze) {
                    results_pop.open()
                } else {
                    App.view.simulation_content_view = true
                    App.view.workflow_phase = ViewModule.Analyze
                }
            }
            onMenuRequested: results_pop.open()
        }
    }

    STComboBox {
        id: compactModeSelector

        visible: root.compact
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4

        model: [
            qsTr("1  Load Scene"),
            qsTr("2  Configure Scene"),
            qsTr("3  Trace Scene"),
            qsTr("4  Analyze Results")
        ]
        currentIndex: Math.max(0, App.view.workflow_phase - 1)
        onActivated: root.selectWorkflow(currentIndex)
    }

    Item {
        anchors.fill: parent

        ResultsPopup {
            id: results_pop

            //y: root.popup_above ? -height - 10 : root.height + 10
            width: root.width
            height: Overlay.overlay.height * 0.66
        }


        DataPopup {
            id: data_pop

            //y: root.popup_above ? -height - 10 : root.height + 10
            width: root.width
            height: Overlay.overlay.height * 0.66
        }
    }
}
