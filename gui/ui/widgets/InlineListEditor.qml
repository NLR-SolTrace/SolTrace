import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STScrollView {
    id: root

    property alias model: item_list.model
    property alias currentIndex: item_list.currentIndex
    property bool editing: false

    property Component listDelegate: null
    property Component listHeader: null
    property Component searchBar: null
    property Component actions: null
    property Component detailView: null
    property bool searchVisible: false

    property string emptyListText: qsTr("No items available")
    property string helpText: qsTr("Select an item to edit its properties.")
    property int visibleRows: 4
    property int rowHeight: 36

    readonly property int list_count: {
        if (!item_list.model) return 0
        return typeof item_list.model.rowCount === "function"
                ? item_list.model.rowCount() : item_list.model.count
    }

    signal itemClicked(int index, var modelData)

    onSearchVisibleChanged: {
        if (searchVisible && search_loader.item)
            search_loader.item.forceActiveFocus()
    }

    function clearSelection() {
        item_list.currentIndex = -1
        editing = false
    }

    contentWidth: availableWidth
    clip: true

    Connections {
        target: item_list.model
        ignoreUnknownSignals: true

        function onRowsRemoved(modelParent, first, last) {
            if (root.list_count === 0) {
                root.clearSelection()
            } else if (root.currentIndex > last) {
                root.currentIndex -= last - first + 1
            } else if (root.currentIndex >= first
                       && root.currentIndex <= last) {
                root.clearSelection()
            }
        }
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: 8

        Loader {
            Layout.fillWidth: true
            sourceComponent: root.listHeader
            active: root.listHeader !== null
            visible: active
        }

        Loader {
            id: search_loader
            Layout.fillWidth: true
            sourceComponent: root.searchBar
            active: root.searchBar !== null
            visible: active && root.searchVisible
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 6

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: root.visibleRows * root.rowHeight + 2
                color: "transparent"
                border.width: 1
                border.color: Material.dividerColor
                radius: 6

                ListView {
                    id: item_list
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    currentIndex: -1
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: STScrollBar { }

                    delegate: Loader {
                        required property int index
                        required property var model
                        width: ListView.view.width
                        height: root.rowHeight
                        sourceComponent: root.listDelegate

                        property int itemIndex: index
                        property var itemModel: model
                        property bool isCurrent: root.currentIndex === index

                        Connections {
                            target: item
                            ignoreUnknownSignals: true

                            function onClicked() {
                                root.currentIndex = index
                                root.editing = true
                                root.itemClicked(index, model)
                            }
                        }
                    }

                    Label {
                        anchors.centerIn: parent
                        anchors.margins: 12
                        width: parent.width - 24
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        text: root.emptyListText
                        opacity: 0.5
                        visible: item_list.count === 0
                    }
                }
            }

            Loader {
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.actions
                active: root.actions !== null
                visible: active
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.margins: 12
            visible: !root.editing
            text: root.helpText
            opacity: 0.65
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }

        Loader {
            Layout.fillWidth: true
            Layout.preferredHeight: item ? item.implicitHeight : 0
            sourceComponent: root.detailView
            active: root.detailView !== null && root.editing
            visible: active
        }
    }
}
