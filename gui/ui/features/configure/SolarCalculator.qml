import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root

    property string error_string: ""
    readonly property var utcOffsetModel: {
        const values = []
        for (let offset = -12; offset <= 14; ++offset) {
            const absolute = Math.abs(offset)
            const hours = absolute < 10 ? "0" + absolute : absolute
            values.push({
                value: offset,
                text: "UTC" + (offset < 0 ? "\u2212" : "+") + hours + ":00"
            })
        }
        return values
    }

    function apply_changes() {
        root.error_string = App.sun.apply_calculator(
            App.sun.calc_data.calculator,
            App.sun.calc_data.latitude,
            App.sun.calc_data.longitude,
            App.sun.calc_data.year,
            App.sun.calc_data.month,
            App.sun.calc_data.day,
            App.sun.calc_data.hour,
            App.sun.calc_data.minute,
            App.sun.calc_data.second,
            App.sun.calc_data.timezone_offset,
            App.sun.calc_data.altitude,
            App.sun.calc_data.pressure,
            App.sun.calc_data.temperature)
    }

    spacing: 10

    STFormPanel {
        title: qsTr("Calculator")

        STFormRow {
            label: qsTr("Method")

            STComboBox {
                Layout.fillWidth: true
                model: [
                    { value: 0, text: qsTr("Legacy") },
                    { value: 1, text: qsTr("Duffie") },
                    { value: 2, text: qsTr("SOLPOS") },
                    { value: 3, text: qsTr("SPA") }
                ]
                textRole: "text"
                valueRole: "value"
                currentValue: App.sun.calc_data.calculator
                onActivated: App.sun.calc_data.calculator = currentValue
            }
        }
    }

    InlineDocumentation {
        key: "configure.sun.calculators." +
             ["legacy", "duffie", "solpos", "spa"][App.sun.calc_data.calculator]
        Layout.fillWidth: true
        Layout.margins: 8
    }

    STFormPanel {
        title: qsTr("Location")

        STFormRow {
            label: qsTr("Latitude (deg)")

            STDoubleSpinBox {
                Layout.fillWidth: true
                from: -90
                to: 90
                decimals: 4
                value: App.sun.calc_data.latitude
                onValueModified: App.sun.calc_data.latitude = value
            }
        }

        STFormRow {
            label: qsTr("Longitude (deg)")

            STDoubleSpinBox {
                Layout.fillWidth: true
                from: -180
                to: 180
                decimals: 4
                value: App.sun.calc_data.longitude
                onValueModified: App.sun.calc_data.longitude = value
            }
        }
    }

    STFormPanel {
        title: qsTr("Date and time")

        STFormRow {
            label: qsTr("Date")

            STComboBox {
                Layout.fillWidth: true
                Layout.minimumWidth: 90
                model: [
                    qsTr("January"), qsTr("February"), qsTr("March"),
                    qsTr("April"), qsTr("May"), qsTr("June"),
                    qsTr("July"), qsTr("August"), qsTr("September"),
                    qsTr("October"), qsTr("November"), qsTr("December")
                ]
                currentIndex: App.sun.calc_data.month - 1
                onActivated: App.sun.calc_data.month = currentIndex + 1

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Month")
            }

            STDoubleSpinBox {
                Layout.minimumWidth: 72
                Layout.preferredWidth: 72
                from: 1
                to: 31
                decimals: 0
                value: App.sun.calc_data.day
                onValueModified: App.sun.calc_data.day = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Day")
            }

            STDoubleSpinBox {
                Layout.minimumWidth: 90
                Layout.preferredWidth: 96
                from: 1900
                to: 2200
                decimals: 0
                value: App.sun.calc_data.year
                onValueModified: App.sun.calc_data.year = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Year")
            }

        }

        STFormRow {
            label: qsTr("Time")

            STDoubleSpinBox {
                Layout.fillWidth: true
                Layout.minimumWidth: 60
                from: 0
                to: 23
                decimals: 0
                value: App.sun.calc_data.hour
                onValueModified: App.sun.calc_data.hour = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Hour")
            }

            Label {
                text: ":"
            }

            STDoubleSpinBox {
                Layout.fillWidth: true
                Layout.minimumWidth: 60
                from: 0
                to: 59
                decimals: 0
                value: App.sun.calc_data.minute
                onValueModified: App.sun.calc_data.minute = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Minute")
            }

            Label {
                text: ":"
            }

            STDoubleSpinBox {
                Layout.fillWidth: true
                Layout.minimumWidth: 60
                from: 0
                to: 59
                decimals: 0
                value: App.sun.calc_data.second
                onValueModified: App.sun.calc_data.second = value

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Second")
            }

        }

        STFormRow {
            label: qsTr("Presets")

            STButton {
                Layout.fillWidth: true
                text: qsTr("Date")
                right_text_icon: "\uf0d7"
                onClicked: datePresetMenu.open()

                STMenu {
                    id: datePresetMenu

                    MenuItem {
                        text: qsTr("Spring (March 20)")
                        onTriggered: App.sun.calc_data.set_spring()
                    }
                    MenuItem {
                        text: qsTr("Summer (June 21)")
                        onTriggered: App.sun.calc_data.set_summer()
                    }
                    MenuItem {
                        text: qsTr("Fall (September 22)")
                        onTriggered: App.sun.calc_data.set_fall()
                    }
                    MenuItem {
                        text: qsTr("Winter (December 21)")
                        onTriggered: App.sun.calc_data.set_winter()
                    }
                }
            }

            STButton {
                Layout.fillWidth: true
                text: qsTr("Time")
                right_text_icon: "\uf0d7"
                onClicked: timePresetMenu.open()

                STMenu {
                    id: timePresetMenu

                    MenuItem {
                        text: qsTr("Dawn (6am)")
                        onTriggered: App.sun.calc_data.set_dawn()
                    }
                    MenuItem {
                        text: qsTr("Mid-Morning (9am)")
                        onTriggered: App.sun.calc_data.set_mid_morning()
                    }
                    MenuItem {
                        text: qsTr("Noon (12pm)")
                        onTriggered: App.sun.calc_data.set_noon()
                    }
                    MenuItem {
                        text: qsTr("Mid-Afternoon (3pm)")
                        onTriggered: App.sun.calc_data.set_mid_afternoon()
                    }
                    MenuItem {
                        text: qsTr("Golden Hour (5pm)")
                        onTriggered: App.sun.calc_data.set_golden_hour()
                    }
                    MenuItem {
                        text: qsTr("Dusk (7pm)")
                        onTriggered: App.sun.calc_data.set_dusk()
                    }
                }
            }
        }

        STFormRow {
            label: qsTr("Time zone")

            STComboBox {
                Layout.fillWidth: true
                model: root.utcOffsetModel
                textRole: "text"
                valueRole: "value"
                currentValue: App.sun.calc_data.timezone_offset
                onActivated: App.sun.calc_data.timezone_offset = currentValue
            }
        }
    }

    STFormPanel {
        Layout.fillWidth: true
        title: qsTr("Atmospheric conditions")
        visible: App.sun.calc_data.calculator === SolarCalculatorData.SPA

        STFormRow {
            label: qsTr("Altitude (m)")

            STDoubleSpinBox {
                Layout.fillWidth: true
                from: -440
                to: 8850
                decimals: 1
                value: App.sun.calc_data.altitude
                onValueModified: App.sun.calc_data.altitude = value
            }
        }

        STFormRow {
            label: qsTr("Pressure (hPa)")

            STDoubleSpinBox {
                Layout.fillWidth: true
                from: 950
                to: 1050
                decimals: 2
                value: App.sun.calc_data.pressure
                onValueModified: App.sun.calc_data.pressure = value
            }
        }

        STFormRow {
            label: qsTr("Temperature (°C)")

            STDoubleSpinBox {
                Layout.fillWidth: true
                from: -50
                to: 60
                decimals: 1
                value: App.sun.calc_data.temperature
                onValueModified: App.sun.calc_data.temperature = value
            }
        }
    }

    Label {
        text: root.error_string
        visible: text.length > 0
        Layout.fillWidth: true
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        color: Material.color(Material.Yellow)
    }

    STButton {
        Layout.fillWidth: true
        text: qsTr("Apply")
        onClicked: root.apply_changes()
    }
}
