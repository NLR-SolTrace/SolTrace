import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SolTrace

Item {
    id: root

    property alias model: internal.model
    property int currentIndex: -1
    property bool editing: false

    property Component listDelegate: null
    property Component listHeader: null
    property Component listFooter: null
    property Component detailView: null
    property string emptyListText: ""

    readonly property bool hasSelection: currentIndex >= 0
    readonly property int list_count: {
        if (!internal.model) return 0
        return typeof internal.model.rowCount === "function"
                ? internal.model.rowCount() : internal.model.count
    }

    signal itemClicked(int index, var modelData)

    function goBack() {
        editing = false
    }

    function clearSelection() {
        currentIndex = -1
        editing = false
    }

    onEditingChanged: {
        if (editing && editor_stack.depth < 2)
            editor_stack.push(detail_component)
        else if (!editing && editor_stack.depth > 1)
            editor_stack.pop()
    }

    Connections {
        target: internal.model
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

    QtObject {
        id: internal
        property var model: null
    }

    StackView {
        id: editor_stack
        anchors.fill: parent
        initialItem: list_component
    }

    Component {
        id: list_component

        ColumnLayout {
            spacing: 0

            Loader {
                Layout.fillWidth: true
                Layout.margins: 8
                sourceComponent: root.listHeader
                active: root.listHeader !== null
                visible: active
            }

            ListView {
                id: list_view
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: internal.model
                ScrollBar.vertical: STScrollBar { }

                delegate: Loader {
                    required property int index
                    required property var model
                    width: ListView.view.width
                    sourceComponent: root.listDelegate

                    property int itemIndex: index
                    property var itemModel: model
                    property bool isCurrent: root.currentIndex === index

                    Connections {
                        target: item
                        function onClicked() {
                            root.currentIndex = index
                            root.editing = true
                            root.itemClicked(index, model)
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    anchors.margins: 24
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                    text: root.emptyListText
                    opacity: 0.5
                    visible: root.emptyListText.length > 0
                             && list_view.count === 0
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.margins: 8
                sourceComponent: root.listFooter
                active: root.listFooter !== null
                visible: active
            }
        }
    }

    Component {
        id: detail_component

        Loader {
            sourceComponent: root.detailView
            active: root.detailView !== null && root.hasSelection
        }
    }
}
