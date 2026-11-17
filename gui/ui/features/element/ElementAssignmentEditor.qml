import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import SolTrace

STFormPanel {
    id: root

    property var module
    title: qsTr("Assignments")

    Label {
        Layout.fillWidth: true

        visible: root.module.current_material_name.length === 0
                 || root.module.current_geometry_name.length === 0
        text: qsTr("Assign a material and geometry for this element to render and participate in a simulation.")
        color: Material.color(Material.Yellow)
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
    }

    STFormRow {
        label: qsTr("Material")

        STButton {
            Layout.fillWidth: true
            property string material_name: root.module.current_material_name
            text: material_name.length ? material_name : qsTr("Unassigned")
            onClicked: material_pop.open()
            SelectItemPopup {
                id: material_pop
                source_model: AppData.materials.materials_list
                onSelectedEntity: (entity) => root.module.current_material = entity
            }
        }
    }

    STFormRow {
        label: qsTr("Geometry")

        STButton {
            Layout.fillWidth: true
            property string geometry_name: root.module.current_geometry_name
            text: geometry_name.length ? geometry_name : qsTr("Unassigned")
            onClicked: geometry_pop.open()
            SelectItemPopup {
                id: geometry_pop
                source_model: AppData.materials.geometry_list
                onSelectedEntity: (entity) => root.module.current_geometry = entity
            }
        }
    }

    STFormRow {
        label: qsTr("Parent")

        STButton {
            Layout.fillWidth: true
            property string parent_name: root.module.parent_name
            text: parent_name.length ? parent_name : qsTr("Unassigned")
            onClicked: parent_pop.open()
            SelectElementPopup {
                id: parent_pop
                acceptEntity: (entity) => root.module.can_set_parent(entity)
                allowNothing: true
                onSelectedElement: (element) => root.module.parent = element
                onSelectedNothing: root.module.clear_parent()
            }
        }
    }
}
