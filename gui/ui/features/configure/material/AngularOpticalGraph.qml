import QtQuick
import QtQuick.Controls.Material
import QtGraphs

import SolTrace

Rectangle {
    id: root

    property var model: null
    property string quantityName: ""
    property color lineColor: Material.color(Material.Blue)

    function rebuild() {
        series.clear()
        if (!root.model) {
            return
        }

        for (let row = 0; row < root.model.rowCount(); ++row) {
            const angle = root.model.data(root.model.index(row, 0))
            const value = root.model.data(root.model.index(row, 1))
            series.append(angle, value)
        }
    }

    onModelChanged: rebuild()
    Component.onCompleted: rebuild()

    Connections {
        target: root.model
        ignoreUnknownSignals: true

        function onChanged() {
            root.rebuild()
        }

        function onModelReset() {
            root.rebuild()
        }
    }

    color: Material.backgroundColor
    radius: 10

    GraphsView {
        id: graph
        anchors.fill: parent
        marginTop: 10
        marginLeft: 20
        marginRight: 6
        marginBottom: 16

        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            backgroundVisible: false
            plotAreaBackgroundVisible: false
            seriesColors: [root.lineColor]
            borderColors: [App.theme.fontColor, App.theme.fontColor]
            grid.mainColor: Qt.alpha(App.theme.fontColor, 0.35)
            grid.subColor: Qt.alpha(App.theme.fontColor, 0.15)
            axisX.mainColor: App.theme.fontColor
            axisX.labelTextColor: App.theme.fontColor
            axisXLabelFont.pixelSize: 9
            axisY.mainColor: App.theme.fontColor
            axisY.labelTextColor: App.theme.fontColor
            axisYLabelFont.pixelSize: 9
            labelTextColor: App.theme.fontColor
        }

        axisX: ValueAxis {
            min: 0
            max: 3141.592
            titleVisible: false
        }

        axisY: ValueAxis {
            min: 0
            max: 1
            titleVisible: false
        }

        LineSeries {
            id: series
            width: 2
            name: ""
        }
    }

    Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 4
        text: qsTr("%1 by incidence angle").arg(root.quantityName)
        font.bold: true
        font.pixelSize: 11
    }

    Label {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Incidence angle (mrad)")
        font.pixelSize: 9
        opacity: 0.75
    }
}
