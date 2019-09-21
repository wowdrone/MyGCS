import QtQuick          2.11
import QtQuick.Controls 2.4

Menu {
    id: menu

    y: toolBar.height

    height: mainWindow.height - toolBar.height
    font.pixelSize: 17
    font.bold: true

    readonly property int menuItemHeight: 70
    readonly property int menuItemWidth: 180
    property bool rollMenu: false

    signal triggerRollEvent()

    background: Rectangle {
        implicitWidth: menuItemWidth
        implicitHeight: menuItemHeight
        color: qgcPal.window
        border.width: 2 * topWindow.sizeFactor; border.color: "#4c4d4f"
    }

    function triggerRoll()
    {
        visible = !rollMenu
        rollMenu = !rollMenu;

        return rollMenu
    }

    MenuItem {
        id: menuItem
        implicitWidth: menuItemWidth
        implicitHeight: menuItemHeight

        text: "Settings"

        background: Rectangle {
            id: backgroundRectangle
            anchors.fill: menu

            opacity: enabled ? 1 : 0.3
            color: menuItem.highlighted ? "darkblue" : "transparent"
        }

        contentItem: Text {
            leftPadding: 10
            rightPadding: 10
            text: menuItem.text
            font: menuItem.font
            opacity: enabled ? 1.0 : 0.3
            color: qgcPal.buttonText
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        onTriggered:
        {
            rollMenu = false
            menu.triggerRollEvent()
        }
    }
}
