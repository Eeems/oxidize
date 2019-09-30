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
        source: "file:/tmp/fb.png"
        cache: false
        function reload(){
            var src = this.source;
            this.source = "";
            this.source = src;
        }
    }
    Keyboard { id: keyboard; visible: false }
}
