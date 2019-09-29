import QtQuick 2.6
import QtQuick.Window 2.3
import "."

Item {
    id: root
    width: screenGeometry.width
    height: screenGeometry.height
    visible: true
    Image {
        anchors.fill: parent
        source: "/temp/fb.png"
    }

    Keyboard { id: keyboard; visible: false }
}
