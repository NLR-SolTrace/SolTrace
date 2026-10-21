import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

Flickable {
    id: root
    property var sun_module : App.sun
    readonly property real radToDeg: 180 / Math.PI
    readonly property real degToRad: Math.PI / 180
    function clamp(value, lo, hi) {
        return Math.max(lo, Math.min(hi, value))
    }

    function directionAzimuth() {
        const x = App.sun.position.x
        const y = App.sun.position.y
        var angle = Math.atan2(x, y) * radToDeg
        while (angle < 0) angle += 360
        while (angle >= 360) angle -= 360
        return angle
    }

    function directionElevation() {
        const x = App.sun.position.x
        const y = App.sun.position.y
        const z = App.sun.position.z
        const length = Math.sqrt(x * x + y * y + z * z)
        if (length <= 1.0e-12) return 0
        return Math.asin(clamp(z / length, -1, 1)) * radToDeg
    }

    function setDirectionAngles(azimuth, elevation) {
        const az = azimuth * degToRad
        const el = elevation * degToRad
        App.sun.position.x = Math.cos(el) * Math.sin(az)
        App.sun.position.y = Math.cos(el) * Math.cos(az)
        App.sun.position.z = Math.sin(el)
        App.sun.position.from_calculator = false
    }

    contentWidth: width
    contentHeight: content_column.implicitHeight
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    ColumnLayout {
        id: content_column
        width: root.width

        InlineDocumentation {
            key: "configure.sun"
            Layout.margins: 8
        }

        Item {
            id: sunPreview
            Layout.fillWidth: true
            Layout.preferredHeight: 240

            property int currentGraph: 0

            Item {
                id: graphHeader
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: Math.max(graphTitle.implicitHeight,
                                 graphControls.implicitHeight)

                Rectangle {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: emissionTypeLabel.implicitWidth + 18
                    height: emissionTypeLabel.implicitHeight + 10
                    radius: height / 2
                    color: Qt.rgba(0, 0, 0, 0.46)
                    border.width: 1
                    border.color: Qt.alpha(App.theme.fontColor, 0.22)

                    Label {
                        id: emissionTypeLabel
                        anchors.centerIn: parent
                        text: App.sun.type === SunModule.PointSource
                              ? qsTr("Point source")
                              : qsTr("Directional sun")
                        color: App.theme.fontColor
                        font.pointSize: App.theme.labelSize
                    }
                }

                Label {
                    id: graphTitle
                    anchors.centerIn: parent
                    text: "Emission Profile"
                    font.bold: true
                }

                Row {
                    id: graphControls
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    STIconButton {
                        icon: sunPreview.currentGraph === 0 ? "\uf1b2" : "\uf201"
                        label: sunPreview.currentGraph === 0 ? "3D" : "2D"
                        toolTip: sunPreview.currentGraph === 0
                                 ? qsTr("Show 3D emission profile")
                                 : qsTr("Show 2D emission profile")
                        onClicked: sunPreview.currentGraph = sunPreview.currentGraph === 0 ? 1 : 0
                    }

                    STIconButton {
                        icon: "\uf1de"
                        toolTip: qsTr("Graph settings")
                        onClicked: graphSettings.open()

                        STPopup {
                            id: graphSettings

                            StackLayout {
                                width: 240
                                currentIndex: sunPreview.currentGraph

                                ColumnLayout {
                                    Label {
                                        text: qsTr("2D graph")
                                        font.bold: true
                                    }

                                    STSpinBoxField {
                                        label: qsTr("Line Width")
                                        Layout.fillWidth: true
                                        value: App.theme.sunShapeGraphLineWidth
                                        onValueChanged: App.theme.sunShapeGraphLineWidth = value
                                        from: 1
                                        to: 10
                                    }

                                    ColorPickerField {
                                        label: qsTr("Color")
                                        Layout.fillWidth: true
                                        color: App.theme.sunShapeGraphLineColor
                                        onColorChanged: App.theme.sunShapeGraphLineColor = color
                                    }
                                }

                                ColumnLayout {
                                    Label {
                                        text: qsTr("3D graph")
                                        font.bold: true
                                    }

                                    RowLayout {
                                        Label {
                                            Layout.fillWidth: true
                                            text: qsTr("Wireframe")
                                        }
                                        STSwitch {
                                            checked: surfaceGraph.wireframeVisible
                                            onToggled: surfaceGraph.wireframeVisible = checked
                                        }
                                    }

                                    RowLayout {
                                        Label {
                                            Layout.fillWidth: true
                                            text: qsTr("Orthographic")
                                        }
                                        STSwitch {
                                            checked: surfaceGraph.orthographicProjection
                                            onToggled: surfaceGraph.orthographicProjection = checked
                                        }
                                    }

                                    RowLayout {
                                        Label {
                                            Layout.fillWidth: true
                                            text: qsTr("Grid")
                                        }
                                        STSwitch {
                                            checked: surfaceGraph.gridVisible
                                            onToggled: surfaceGraph.gridVisible = checked
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            StackLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: graphHeader.bottom
                anchors.bottom: parent.bottom
                currentIndex: sunPreview.currentGraph

                SunShapeGraph {
                    title: "Emission Profile"
                    xAxisTitle: "Angle (mrad)"
                    yAxisTitle: "Intensity"
                    headerVisible: false
                }

                SunPreview {
                    id: surfaceGraph
                }
            }
        }

        STTabBar {
            id: bar
            secondary: true
            currentIndex: App.view.sun_section
            onCurrentIndexChanged: App.view.sun_section = currentIndex
            Layout.fillWidth: true
            Layout.leftMargin: 12
            model: ["Type & Position", "Emission Profile"]
        }

        StackLayout {
            currentIndex: App.view.sun_section

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        id: directionalSunTypeLabel
                        Layout.rightMargin: 10
                        Layout.preferredWidth: directionalSunPositionLabel.implicitWidth
                        text: "Type "
                    }

                    STComboBox {
                        Layout.fillWidth: true
                        model: [
                            { value: 0, text: "Directional Sun" },
                            { value: 1, text: "Point Source" }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        currentValue: App.sun.type
                        onActivated: App.sun.type = currentValue
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: App.sun.type === SunModule.Directional

                    Label {
                        id: directionalSunPositionLabel
                        Layout.rightMargin: 10
                        text: "Position"
                    }

                    STComboBox {
                        Layout.fillWidth: true
                        model: [
                            { value: 0, text: "Solar Calculator" },
                            { value: 1, text: "Manual" }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        currentValue: App.sun.ds_position_type
                        onActivated: App.sun.ds_position_type = currentValue
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Material.dividerColor
                }

                InlineDocumentation {
                    Layout.fillWidth: true
                    Layout.margins: 8
                    key: "configure.sun.type." + ["directional", "point_source"][App.sun.type]
                }

                ColumnLayout {

                    SolarCalculator {
                        Layout.fillWidth: true

                        visible: (App.sun.type === SunModule.Directional
                                  && App.sun.ds_position_type === 0)
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 1
                        visible: App.sun.type === SunModule.Directional

                        STSpinBoxField {
                            id: azimuthField
                            Layout.fillWidth: true
                            Layout.preferredWidth: 100
                            label: "Azimuth (deg)"
                            value: root.directionAzimuth()
                            from: 0
                            to: 360
                            decimals: 3
                            enabled: App.sun.ds_position_type === SunModule.Angle
                            onValueModified: root.setDirectionAngles(
                                                 value,
                                                 root.directionElevation())
                        }

                        STSpinBoxField {
                            id: elevationField
                            Layout.fillWidth: true
                            Layout.preferredWidth: 100
                            label: "Elevation (deg)"
                            value: root.directionElevation()
                            from: -90
                            to: 90
                            decimals: 3
                            enabled: App.sun.ds_position_type === SunModule.Angle
                            onValueModified: root.setDirectionAngles(
                                                 root.directionAzimuth(),
                                                 value)
                        }
                    }
                }

                STPropertyPanel {
                    Layout.fillWidth: true
                    title: "Manual Position"
                    collapsible: false
                    visible: App.sun.type === SunModule.PointSource

                    GridLayout {
                        width: parent.width

                        STSpinBoxField {
                            Layout.row: 0
                            Layout.column: 0
                            Layout.fillWidth: true
                            Layout.preferredWidth: 100
                            value: App.sun.position.x
                            onValueModified: {
                                App.sun.position.x = value
                                App.sun.position.from_calculator = false
                            }
                            label: "X"
                            from: -1000000
                            to: 1000000
                            decimals: 4
                        }

                        STSpinBoxField {
                            Layout.row: 1
                            Layout.column: 0
                            Layout.fillWidth: true
                            Layout.preferredWidth: 100
                            label: "Y"
                            value: App.sun.position.y
                            onValueModified: {
                                App.sun.position.y = value
                                App.sun.position.from_calculator = false
                            }
                            from: -1000000
                            to: 1000000
                            decimals: 4
                        }

                        STSpinBoxField {
                            Layout.row: 2
                            Layout.column: 0
                            Layout.fillWidth: true
                            Layout.preferredWidth: 100
                            label: "Z"
                            value: App.sun.position.z
                            onValueModified: {
                                App.sun.position.z = value
                                App.sun.position.from_calculator = false
                            }
                            from: -1000000
                            to: 1000000
                            decimals: 4
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "Shape Type:"
                    }

                    STComboBox {
                        Layout.fillWidth: true
                        model: [
                            { value: 0, text: "Gaussian" },
                            { value: 1, text: "Pillbox" },
                            { value: 2, text: "CSR" },
                            { value: 3, text: "Custom" },
                            { value: 4, text: "Limb Darkened" }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        currentValue: App.sun.shape.shape
                        onActivated: App.sun.shape.shape = currentValue
                    }
                }

                InlineDocumentation {
                    Layout.fillWidth: true
                    Layout.margins: 8
                    key: "configure.sun.shape." + ["gaussian", "pillbox", "csr", "custom", "limb_darkened"][App.sun.shape.shape]
                }

                GridLayout {
                    columns: 1
                    Layout.margins: 12

                    StackLayout {
                        currentIndex: App.sun.shape.shape
                        Layout.row: 0
                        Layout.column: 0
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 200

                        STSpinBoxField {
                            Layout.fillWidth: true
                            label: "Standard Deviation (mrad)"
                            value: App.sun.shape.sigma
                            decimals: 3
                            onValueModified: App.sun.shape.sigma = value
                        }

                        STSpinBoxField {
                            Layout.fillWidth: true
                            label: "Half-width (mrad)"
                            value: App.sun.shape.half_width
                            decimals: 3
                            onValueModified: App.sun.shape.half_width = value
                        }

                        STSpinBoxField {
                            Layout.fillWidth: true
                            label: "Circumsolar Ratio"
                            value: App.sun.shape.csr
                            decimals: 3
                            from: 0
                            to: 0.8
                            stepSize: 0.1
                            onValueModified: App.sun.shape.csr = value
                        }

                        CustomSunShapeTable { }

                        Label {
                            Layout.fillWidth: true
                            text: "No parameters"
                            horizontalAlignment: Text.AlignHCenter
                            opacity: 0.7
                        }
                    }

                }
            }
        }
    }

    ScrollBar.vertical: STScrollBar { }
}
