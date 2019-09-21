import QtQuick 2.6
import QtQuick.Window 2.0
import Qt.labs.folderlistmodel 1.0
import "../widget"

Item {
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
    FileList {
        id: fileList
        cwd: "/"
        anchors.top: topbar.bottom
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.right: root.right
        width: screenGeometry.width
        height: root.height - topbar.height
        onFileClick: function(file) {
            if(file.isDir){
                pathInput.clear();
                pathInput.text = file.path;
                fileList.cwd = file.path
                console.log("chdir " + cwd);
            }else{
                console.log("open " + file.path);
            }
        }
        onScrollStart: keyboard.hide()
        onTouchStart: keyboard.hide()
    }
    Rectangle {
        id: topbar
        color: "black"
        width: root.width
        height: 100
        Button {
            id: upButton
            text: "⬅️"
            fontsize: 10
            width: 100
            height: 100
            anchors.left: parent.left
            color: "white"
            backgroundColor: "transparent"
            borderColor: "transparent"
            selectedColor: "black"
            selectedBackgroundColor: "white"
            selectedBorderColor: "transparent"
            onClick: {
                if(fileList.cwd === "/"){
                    root.visible = false;
                    quitTimer.start();
                    return;
                }
                var parts = fileList.cwd.split("/");
                parts.pop();
                var path = parts.join("/");
                if(path === ""){
                    path = "/";
                }
                pathInput.clear();
                pathInput.text = path;
                fileList.cwd = path;
                console.log("chdir " + cwd);
            }
        }
        TextInput {
            id: pathInput
            text: cwd
            padding: 30
            width: root.width - upButton.width
            height: 100
            anchors.left: upButton.right
            color: "white"
            clip: true
            onFocusChanged: {
                keyboard.visible = this.focus;
            }
            onAccepted: {
                fileList.cwd = text
                this.focus = false
            }
        }
    }
    Rectangle {
        color: "white"
        x: parseInt(pathInput.x + pathInput.leftPadding)
        y: parseInt(pathInput.y + pathInput.height - pathInput.bottomPadding)
        width: parseInt(pathInput.width - pathInput.leftPadding - pathInput.rightPadding)
        height: 1
    }
    Keyboard { id: keyboard; visible: false }
}
