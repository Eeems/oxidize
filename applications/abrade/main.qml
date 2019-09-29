import QtQuick 2.6
import QtQuick.Window 2.3
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Rectangle {
    id: root
    visible: true
    width: screenGeometry.width
    height: screenGeometry.height
    color: "transparent"
    Item { }
    Keyboard { id: keyboard; visible: false }
}
