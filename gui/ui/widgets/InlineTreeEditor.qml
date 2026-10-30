import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STScrollView {
    id: root

    property alias model: item_tree.model
    property bool editing: false
    property var selectedEntity

    property Component searchBar: null
    property Component actions: null
    property Component detailView: null
    property bool searchVisible: false

    property string emptyListText: qsTr("No items available")
    property string helpText: qsTr("Select an item to edit its properties.")
    property int visibleRows: 6
    property int rowHeight: 36

    readonly property int visibleRowCount: item_tree.rows

    signal itemClicked(var entity)

    onSearchVisibleChanged: {
        if (searchVisible && search_loader.item)
            search_loader.item.forceActiveFocus()
    }

    function clearSelection() {
        editing = false
    }

    function revealEntity(entity) {
        if (!entity || !entity.is_valid() || !item_tree.model
                || typeof item_tree.model.index_for_entity !== "function") {
            return
        }

        Qt.callLater(function() {
            const modelIndex = item_tree.model.index_for_entity(entity)
            if (!modelIndex || modelIndex.row < 0) return

            item_tree.expandToIndex(modelIndex)
            const row = item_tree.rowAtIndex(modelIndex)
            if (row >= 0)
                item_tree.positionViewAtRow(row, TableView.Contain)
        })
    }

    function expandAll() {
        if (!item_tree.model) return

        Qt.callLater(function() {
            const count = item_tree.model.rowCount()
            for (let rootRow = 0; rootRow < count; ++rootRow) {
                const modelIndex = item_tree.model.index(rootRow, 0)
                const viewRow = item_tree.rowAtIndex(modelIndex)
                if (viewRow >= 0)
                    item_tree.expandRecursively(viewRow)
            }
        })
    }

    contentWidth: availableWidth
    clip: true

    ColumnLayout {
        width: root.availableWidth
        spacing: 8

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

                TreeView {
                    id: item_tree
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: STScrollBar { }

                    delegate: TreeViewDelegate {
                        id: tree_delegate

                        required property var entity

                        implicitHeight: root.rowHeight
                        implicitWidth: item_tree.width
                        width: item_tree.width
                        highlighted: entity === root.selectedEntity

                        onClicked: {
                            root.editing = true
                            root.itemClicked(entity)
                        }

                        background: Rectangle {
                            opacity: tree_delegate.enabled ? 1 : 0.3
                            color: tree_delegate.down
                                   ? Material.rippleColor
                                   : tree_delegate.highlighted
                                     ? Material.highlightedRippleColor
                                     : tree_delegate.hovered
                                       ? Qt.alpha(Material.highlightedRippleColor, 0.35)
                                       : "transparent"
                            radius: 14
                        }
                    }

                }

                Label {
                    anchors.centerIn: parent
                    width: Math.max(0, parent.width - 24)
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                    text: root.emptyListText
                    opacity: 0.5
                    visible: root.visibleRowCount === 0
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
