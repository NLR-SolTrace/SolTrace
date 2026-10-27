pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ComboBox {
    id: root

    property int lastIndex: currentIndex < 0 ? 0 : currentIndex

    Layout.preferredWidth: Math.max(implicitWidth + 50, 200)
    Material.foreground: App.theme.fontColor
    font.pointSize: App.theme.labelSize

    model: AppData.color_maps
    textRole: "name"
    valueRole: "id"

    onCurrentIndexChanged: {
        if (currentIndex < 0) currentIndex = lastIndex
        else lastIndex = currentIndex
    }

    background: WellRectangle {
        implicitWidth: 160
        implicitHeight: 32
        radius: height / 2
    }

    contentItem: RowLayout {
        spacing: 8

        Image {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 12
            source: AppData.color_maps.source_for(root.currentValue)
            fillMode: Image.Stretch
            smooth: true
        }

        Label {
            Layout.fillWidth: true
            text: root.displayText
            font: root.font
            color: App.theme.fontColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
    }

    indicator: Label {
        font.family: "Font Awesome 7 Free"
        text: "\uf078"

        x: root.width - width - root.rightPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        width: 12
        height: 12
    }

    popup: Popup {
        width: Math.max(root.width, 260)
        height: Math.min(contentItem.implicitHeight,
                         root.Window.height - topMargin - bottomMargin)
        padding: 2

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex

            ScrollBar.vertical: STScrollBar { }

            delegate: STItemDelegate {
                id: delegate
                required property var model
                required property int index

                width: ListView.view.width
                text: model.name

                background: Rectangle {
                    implicitHeight: 32
                    opacity: enabled ? 1 : 0.3
                    color: parent.down
                           ? Material.rippleColor : "transparent"
                }

                contentItem: RowLayout {
                    spacing: 10

                    Image {
                        Layout.preferredWidth: 96
                        Layout.preferredHeight: 14
                        source: delegate.model.source
                        fillMode: Image.Stretch
                        smooth: true
                    }

                    Label {
                        Layout.fillWidth: true
                        text: delegate.text
                        font: delegate.font
                        elide: Text.ElideRight
                    }
                }
            }
        }

        background: Rectangle {
            color: Qt.alpha(Material.backgroundColor, .95)
            border.color: Material.frameColor
            radius: 10
        }
    }
}
