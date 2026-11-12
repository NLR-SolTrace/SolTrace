import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import SolTrace

ColumnLayout {
    id: root

    property var group_edit: App.materials.geometry_edit

    spacing: 8

    component PropertySection : STPropertyPanel {
        id: section
        required property var parameterModel
        required property string typeLabel
        required property var typeModel
        required property int currentKind
        signal kindChanged(int kind)

        Layout.fillWidth: true
        collapsible: true
        columns: 1

        STPropertyLabel {
            text: section.typeLabel
            Layout.row: 0
            Layout.column: 0
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

        STComboBox {
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            Layout.row: 1
            Layout.column: 0
            model: section.typeModel
            textRole: "name"
            valueRole: "value"
            currentValue: section.currentKind
            onActivated: section.kindChanged(currentValue)
        }

        Repeater {
            model: section.parameterModel
            delegate: STPropertyLabel {
                required property int index
                required property var model
                required property string name
                required property real content
                required property int type
                readonly property bool isFlatFocalLength: type === 2
                                                               && (content === 0
                                                                   || !Number.isFinite(content))
                text: name
                      + (isFlatFocalLength ? qsTr(" (flat)") : "")
                      + (!model.editable ? qsTr(" (set by surface)") : "")
                Layout.row: index * 2 + 2
                Layout.column: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
        }

        Repeater {
            model: section.parameterModel
            delegate: STDoubleSpinBox {
                required property int index
                required property var model
                required property real content
                required property real min
                required property real max
                required property int type
                readonly property bool isAngle: type === 1
                readonly property bool isFocalLength: type === 2

                function toDisplayValue(value) {
                    if (isFocalLength && value === 0) return Infinity
                    return isAngle ? value * 180.0 / Math.PI : value
                }

                function toModelValue(value) {
                    if (isFocalLength && !Number.isFinite(value)) return 0
                    return isAngle ? value * Math.PI / 180.0 : value
                }

                Layout.fillWidth: true
                Layout.row: index * 2 + 3
                Layout.column: 0
                enabled: model.editable
                value: toDisplayValue(content)
                from: toDisplayValue(min)
                to: toDisplayValue(max)
                decimals: 8
                stepSize: isAngle ? 1 : (isFocalLength ? 0.1 : 0.01)
                //suffix: isAngle ? "deg" : ""
                onValueModified: model.content = toModelValue(value)
            }
        }
    }

    PropertySection {
        title: qsTr("Surface Properties")
        typeLabel: qsTr("Surface Type")
        parameterModel: root.group_edit.surface_parameter_model
        typeModel: root.group_edit.surface_parameter_model.surface_type_model
        currentKind: root.group_edit.surface_parameter_model.surface_kind
        onKindChanged: (kind) => root.group_edit.surface_parameter_model.surface_kind = kind
    }

    PropertySection {
        title: qsTr("Aperture Properties")
        typeLabel: qsTr("Aperture Type")
        parameterModel: root.group_edit.aperture_parameter_model
        typeModel: root.group_edit.aperture_parameter_model.aperture_type_model
        currentKind: root.group_edit.aperture_parameter_model.aperture_kind
        onKindChanged: (kind) => root.group_edit.aperture_parameter_model.aperture_kind = kind
    }
}
