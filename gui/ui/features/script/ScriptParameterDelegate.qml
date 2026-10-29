pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

Item {
    id: root

    required property string name
    required property string type
    required property string description
    required property var choices
    required property bool min_bounded
    required property bool max_bounded
    required property bool max_inclusive
    required property real min
    required property real max
    required property real step
    required property int decimals
    required property string unit
    required property string value
    required property var model

    implicitHeight: content.implicitHeight + 12

    function commitVector(x, y, z) {
        model.value = "[" + String(x) + "," + String(y) + "," + String(z) + "]"
    }

    RowLayout {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: root.name
                font.bold: true
                elide: Label.ElideRight
            }

            Label {
                visible: root.description.length > 0
                Layout.fillWidth: true
                text: root.description
                opacity: 0.65
                font.pointSize: Math.max(8, App.theme.labelSize - 1)
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            sourceComponent: {
                if (root.type === "string" && root.choices.length > 0)
                    return choiceEditor
                if (root.type === "integer"
                        && (root.min_bounded || root.max_bounded))
                    return integerEditor
                if (root.type === "real"
                        && (root.min_bounded || root.max_bounded))
                    return realEditor
                if (root.type === "vec3")
                    return vectorEditor
                return textEditor
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Material.dividerColor
        opacity: 0.55
    }

    Component {
        id: textEditor

        STTextField {
            text: root.value
            onAccepted: root.model.value = text
            onTextEdited: root.model.value = text
        }
    }

    Component {
        id: choiceEditor

        STComboBox {
            model: root.choices
            currentIndex: Math.max(0, root.choices.indexOf(root.value))
            onActivated: root.model.value = currentText
        }
    }

    Component {
        id: integerEditor

        STSpinBox {
            from: root.min_bounded
                  ? Math.max(-2147483648, Math.ceil(root.min))
                  : -2147483648
            to: root.max_bounded
                ? Math.min(2147483647,
                           Math.floor(root.max) - (root.max_inclusive ? 0 : 1))
                : 2147483647
            stepSize: Math.max(1, Math.round(root.step))
            value: Number(root.value)
            onValueModified: root.model.value = String(value)
        }
    }

    Component {
        id: realEditor

        STDoubleSpinBox {
            from: root.min_bounded ? root.min : -Infinity
            to: root.max_bounded ? root.max : Infinity
            stepSize: root.step
            decimals: root.decimals
            value: Number(root.value)
            onValueModified: root.model.value = String(value)
        }
    }

    Component {
        id: vectorEditor

        STButton {
            id: vector_button

            function loadEditor() {
                const components = root.value.replace(/[\[\]{}]/g, "")
                                             .split(/[\s,]+/)
                                             .filter(part => part.length > 0)
                                             .map(Number)
                if (components.length === 3
                        && components.every(Number.isFinite)) {
                    vector_x.value = components[0]
                    vector_y.value = components[1]
                    vector_z.value = components[2]
                }
            }

            text: root.value
            right_text_icon: "\uf304"
            onClicked: {
                loadEditor()
                vector_popup.open()
            }

            STPopup {
                id: vector_popup

                parent: Overlay.overlay
                width: 300
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                enable_shadow: true

                x: {
                    const point = vector_button.mapToItem(parent, 0, 0)
                    return Math.max(8, Math.min(point.x, parent.width - width - 8))
                }
                y: {
                    const point = vector_button.mapToItem(
                                parent, 0, vector_button.height + 4)
                    return Math.max(8, Math.min(point.y, parent.height - height - 8))
                }

                contentItem: ColumnLayout {
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        text: root.unit.length > 0
                              ? root.name + " (" + root.unit + ")"
                              : root.name
                        font.bold: true
                        elide: Label.ElideRight
                    }

                    STFormRow {
                        label: "X"

                        STDoubleSpinBox {
                            id: vector_x
                            Layout.fillWidth: true
                            from: -Infinity
                            to: Infinity
                            stepSize: root.step
                            decimals: root.decimals
                            onValueModified: root.commitVector(
                                                 value,
                                                 vector_y.value,
                                                 vector_z.value)
                        }
                    }

                    STFormRow {
                        label: "Y"

                        STDoubleSpinBox {
                            id: vector_y
                            Layout.fillWidth: true
                            from: -Infinity
                            to: Infinity
                            stepSize: root.step
                            decimals: root.decimals
                            onValueModified: root.commitVector(
                                                 vector_x.value,
                                                 value,
                                                 vector_z.value)
                        }
                    }

                    STFormRow {
                        label: "Z"

                        STDoubleSpinBox {
                            id: vector_z
                            Layout.fillWidth: true
                            from: -Infinity
                            to: Infinity
                            stepSize: root.step
                            decimals: root.decimals
                            onValueModified: root.commitVector(
                                                 vector_x.value,
                                                 vector_y.value,
                                                 value)
                        }
                    }
                }
            }
        }
    }
}
