import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SolTrace

Item {
    id: root

    property alias model: internal.model

    property bool editing: false

    // Header component shown above the list (e.g. a search field)
    property Component listHeader: null

    // Footer component shown below the list (e.g. an add button)
    property Component listFooter: null

    // The component shown when an element is selected
    property Component detailView: null

    // Text shown over the list when the list model is empty
    property string emptyListText: ""

    // Signal
    signal itemClicked(db_entity entity)

    onEditingChanged: {
        if (root.editing && editorStack.depth < 2)
            editorStack.push(detailComponent)
        else if (!root.editing && editorStack.depth > 1)
            editorStack.pop()
    }

    function goBack() {
        editing = false
    }

    function clearSelection() {
        editing = false
    }

    Connections {
        target: internal.model
        ignoreUnknownSignals: true
        function onRowsRemoved(modelParent, first, last) {
            root.editing = false
        }
    }

    QtObject {
        id: internal
        property var model: null
    }

    StackView {
        id: editorStack
        anchors.fill: parent
        initialItem: listComponent
    }

    Component{
        id: listDelegate

        STItemDelegate {
            id: st_del_root
            required property int index

            required property string name
            required property var entity
            required property bool has_children

            signal opened(db_entity entity)

            text: name

            highlighted: entity === AppData.layout.edited_element

            onClicked: {
                st_del_root.ListView.view.editorRoot.editing = true
                st_del_root.ListView.view.editorRoot.itemClicked(entity)
            }

            onOpened: {
                st_del_root.ListView.view.editorRoot.editing = false
            }

            STIconButton {
                icon: "\uf802"
                visible: st_del_root.has_children

                onClicked: {
                    AppData.layout.viewed_element = st_del_root.entity
                    AppData.layout.clear_edited_element()
                    st_del_root.opened(st_del_root.entity)
                }

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Component {
        id: listComponent

        ColumnLayout {
            spacing: 0

            Breadcrumb {
                Layout.fillWidth: true
                Layout.margins: 3

                onItemSelected: {
                    root.editing = true
                }

                onItemDeselected: {
                    root.editing = false
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.margins: 8

                sourceComponent: root.listHeader
                active: root.listHeader !== null
                visible: active
            }

            ListView {
                id: listView
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: internal.model
                property var editorRoot: root
                ScrollBar.vertical: STScrollBar { }

                delegate: listDelegate

                Label {
                    anchors.centerIn: parent
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                    text: root.emptyListText
                    opacity: 0.5
                    visible: root.emptyListText.length > 0
                             && listView.count === 0
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
        id: detailComponent

        Loader {
            sourceComponent: root.detailView
            active: root.detailView !== null
        }
    }
}
