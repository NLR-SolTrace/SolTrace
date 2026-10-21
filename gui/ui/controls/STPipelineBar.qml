import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

Rectangle {
    id: bar

    property int currentIndex: 0

    property string fontFamily
    property int fontWeight: Font.Normal

    property var abbreviatedModel: []
    property var prefixModel: []
    property var model: []

    height: 42
    radius: height / 2
    color: App.theme.glassColor

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: "transparent"
        border.color: Qt.alpha("black", .25)

        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: "transparent"
            border.color: Qt.alpha("black", .1)
        }
    }

    Label {
        id: default_label
        visible: false
    }

    function select(idx) {
        bar.currentIndex = idx
    }

    function modelCount() {
        if (!bar.model) return 0
        if (bar.model.length !== undefined) return bar.model.length
        if (bar.model.count !== undefined) return bar.model.count
        return 0
    }

    function abbreviatedLabelAt(index) {
        if (bar.abbreviatedModel
                && bar.abbreviatedModel.length > index
                && bar.abbreviatedModel[index]) {
            return bar.abbreviatedModel[index]
        }

        return bar.model[index] ?? ""
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        spacing: 0

        Repeater {
            model: Math.max(0, bar.modelCount() * 2 - 1)

            delegate: Item {
                property bool isArrow: index % 2 === 1
                property int itemIndex: Math.floor(index / 2)

                Layout.fillWidth: !isArrow
                Layout.fillHeight: true
                Layout.preferredWidth: isArrow ? arrowText.implicitWidth + 8 : -1

                Label {
                    id: arrowText
                    visible: isArrow
                    anchors.centerIn: parent
                    font.family: "Font Awesome 7 Free"
                    font.pointSize: App.theme.defaultComboBarTextSize - 2
                    text: "\uf178"
                    opacity: 0.3
                }

                RowLayout {
                    visible: !isArrow
                    anchors.centerIn: parent
                    spacing: 6

                    Label {
                        visible: bar.prefixModel.length > itemIndex
                        Layout.alignment: Qt.AlignVCenter

                        text: bar.prefixModel.length > itemIndex
                            ? bar.prefixModel[itemIndex] : ""
                        font.bold: itemIndex === bar.currentIndex
                        opacity: itemIndex === bar.currentIndex ? 1.0
                            : itemMouse.containsMouse ? 0.75
                            : 0.5
                    }

                    Rectangle {
                        visible: bar.prefixModel.length > itemIndex
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 16
                        Layout.alignment: Qt.AlignVCenter
                        color: Material.dividerColor
                    }

                    Label {
                        font.family: bar.fontFamily.length
                            ? bar.fontFamily : default_label.font.family
                        font.pointSize: App.theme.defaultComboBarTextSize
                        font.weight: itemIndex === bar.currentIndex
                            ? Font.Bold : bar.fontWeight
                        text: bar.abbreviatedLabelAt(itemIndex)
                        opacity: itemIndex === bar.currentIndex ? 1.0
                            : itemMouse.containsMouse ? 0.75
                            : 0.5
                    }

                }

                MouseArea {
                    id: itemMouse
                    visible: !isArrow
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: bar.select(itemIndex)

                    STToolTip {
                        visible: itemMouse.containsMouse
                                 && bar.abbreviatedLabelAt(itemIndex)
                                    !== (bar.model[itemIndex] ?? "")
                        text: bar.model[itemIndex] ?? ""
                    }
                }
            }
        }
    }
}
