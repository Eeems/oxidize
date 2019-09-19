import QtQuick 2.6
import "../widget"

Rectangle {
    id: root
    visible: visible
    width: screenGeometry.width
    height: screenGeometry.height

    Button {
        id: upButton
        text: ".."
        width: 100
        height: 100
        anchors.left: parent.left
        onClick: {
            console.log("Testing")
        }
    }
    Rectangle {
        height: 100
        width: 500
        anchors.left: upButton.right
        border.color: "black"
        border.width: 3
        TextInput {
            text: "$ " + cwd
            anchors.centerIn: parent
        }
    }
    Button {
        text: "X"
        width: 100
        height: 100
        anchors.right: parent.right
        onClick: {
            for (var i = 0; i < root.children.length; i++){
                root.children[i].visible = false;
            }
            quitTimer.start()
        }
    }
    Timer {
        id: quitTimer
        interval: 1000
        repeat: false
        onTriggered: {
            Qt.callLater(Qt.quit());
        }
    }
}
