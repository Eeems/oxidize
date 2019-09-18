import QtQuick 2.6
import QtQuick.Window 2.2

Rectangle {
    id: root
    visible: true
    width: screenGeometry.width
    height: screenGeometry.height

    Text {
        anchors.centerIn: parent
        text: "Hello, World!"
    }
    Text {
        anchors {
            left: parent.left
            top: parent.top
            margins: 32
        }
    }

}
