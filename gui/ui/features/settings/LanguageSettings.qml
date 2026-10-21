import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import SolTrace

STScrollView {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentWidth: availableWidth

    ColumnLayout {
        width: root.availableWidth
        spacing: 12

        Header {
            text: qsTr("Language")
        }

        ColumnLayout {
            STComboBox {
                currentIndex: App.docs.locale
                model: [qsTr("English"), qsTr("Spanish")]
                onActivated: (index) => App.docs.locale = index
            }
        }
    }
}
