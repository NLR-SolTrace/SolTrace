import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import SolTrace

ColumnLayout {
    id: root

    Binding {
        target: bar
        property: "currentIndex"
        value: App.view.configure_section
    }

    InlineDocumentation {
        key: "workflow.configure"
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.bottomMargin: 8
    }

    STTabBar {
        id: bar
        currentIndex: App.view.configure_section
        onCurrentIndexChanged: App.view.configure_section = currentIndex

        Layout.fillWidth: true

        model: ["Ray Source", "Materials", "Geometries", "Staging"]
        
    }
    
    StackLayout {
        currentIndex: App.view.configure_section

        ConfigureSun {
        }

        ConfigureMaterials {
        }
        
        ConfigureGeometry {
        }

        ConfigureLayout {
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 1
        color: Material.dividerColor
    }
    
    WorkflowStepper {
        Layout.fillWidth: true
        previous: "Load Scene"
        next: "Trace Scene"
        currentIndex: ViewModule.Configure
    }
}
