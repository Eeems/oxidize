import QtQuick 2.6
import "../widget"

Rectangle {
    id: root
    visible: visible
    width: screenGeometry.width
    height: screenGeometry.height
    Timer {
        id: quitTimer
        interval: 1000
        repeat: false
        onTriggered: {
            Qt.quit();
        }
    }
    MouseArea {
        anchors.fill: root
        onClicked: {
            focus = true;
            keyboard.visible = false;
        }
    }
    Keyboard { id: keyboard; visible: false }
    Row {
        Button {
            id: upButton
            text: ".."
            width: 100
            height: 100
            onClick: {
                console.log("Testing")
            }
        }
        Rectangle {
            width: root.width - 200
            height: 100
            border.color: "black"
            border.width: 3
            TextInput {
                text: cwd
                anchors.fill: parent
                padding: 30
                onFocusChanged: {
                    keyboard.visible = this.focus;
                }
                onAccepted: {
                    cwd = text
                    this.focus = false
                }
            }
        }
        Button {
            text: "X"
            width: 100
            height: 100
            onClick: {
                root.visible = false
//                for (var i = 0; i < root.children.length; i++){
//                    root.children[i].visible = false;
//                }
                quitTimer.start()
            }
        }

    }
}
