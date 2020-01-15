import QtQuick 2.6
import QtQuick.Window 2.3
import "."

Item {
    id: root
    width: screenGeometry.width
    height: screenGeometry.height
    visible: true
    Image {
        id: background
        objectName: "background"
        anchors.fill: parent
        cache: false
        source: "file:/tmp/fb.png"
        function reload(){
            this.source = "";
            timer.start();
        }
    }
    Timer {
        id: timer
        interval: 1
        onTriggered: background.source = "file:/tmp/fb.png"
    }

    Keyboard { id: keyboard; visible: false }
}
