pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

Item {
    id: bar

    property int currentIndex: 0
    property bool secondary: false

    property string fontFamily
    property int fontWeight: Font.Normal

    property var model: []

    implicitHeight: secondary ? 28 : 32
    height: implicitHeight

    Label {
        id: defaultLabel
        visible: false
    }

    function select(index) {
        bar.currentIndex = index
    }

    function modelCount() {
        if (!bar.model) return 0
        if (bar.model.length !== undefined) return bar.model.length
        if (bar.model.count !== undefined) return bar.model.count
        return 0
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        Rectangle {
            anchors.fill: parent

            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 1.0; color: App.theme.glassColor }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            z: 1
            color: Qt.alpha(Material.foreground, 0.3)
        }

        RowLayout {
            anchors.fill: parent
            anchors.bottomMargin: 1
            spacing: 2

            Repeater {
                model: bar.modelCount()

                delegate: TabButton {
                    id: tab

                    required property int index
                    readonly property int itemIndex: index

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    hoverEnabled: true
                    checkable: true
                    checked: itemIndex === bar.currentIndex
                    text: bar.model[itemIndex] ?? ""
                    onClicked: bar.select(itemIndex)

                    background: Rectangle {
                        color: tab.checked
                               ? Qt.alpha(Material.foreground, 0.1)
                               : tab.hovered
                                 ? Material.rippleColor
                                 : Qt.alpha(App.theme.glassColor, 0.24)
                        border.width: 1
                        border.color: tab.checked
                                      ? Qt.alpha(Material.foreground, 0.45)
                                      : Qt.alpha(Material.dividerColor, 0.45)
                        topLeftRadius: 5
                        topRightRadius: 5

                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }

                        Behavior on border.color {
                            ColorAnimation { duration: 100 }
                        }

                        Rectangle {
                            visible: tab.checked
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 2
                            color: Material.foreground
                        }
                    }

                    contentItem: Label {
                        font.family: bar.fontFamily.length
                                     ? bar.fontFamily
                                     : defaultLabel.font.family
                        font.pointSize: Math.max(
                                            8,
                                            App.theme.comboBarTextSize
                                            - (bar.secondary ? 1 : 0))
                        font.weight: tab.checked ? Font.Bold : bar.fontWeight
                        text: tab.text
                        opacity: tab.checked ? 1.0 : tab.hovered ? 0.82 : 0.62
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
