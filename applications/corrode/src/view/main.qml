import QtQuick 2.6
import QtQuick.Window 2.3
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
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
    Item {
        id: mainArea
        anchors.top: topbar.bottom
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.right: root.right
        width: screenGeometry.width
        height: root.height - topbar.height
        FileList {
            id: fileList
            cwd: "/"
            enabled: this.visible
            anchors.fill: parent
            onScrollStart: keyboard.hide()
            onFileClick: function(file) {
                if(file.isDir){
                    pathInput.clear();
                    pathInput.text = file.path;
                    fileList.cwd = file.path
                    console.log("chdir " + cwd);
                }else{
                    fileInfo.load(file);
                }
            }
            onFileLongPress: function(file){
                fileInfo.load(file);
            }
        }
        Rectangle {
            id: fileInfo
            visible: false
            enabled: this.visible
            anchors.fill: parent
            color: "white"
            function load(file){
                filePath.text = "Path: " + file.path;
                fileUrl.text = "Url: " + file.url;
                fileSize.text = "Size: " + file.humanSize;
                fileModified.text = "Last Modified: " + file.modified;
                fileAccessed.text = "Last Accessed: " + file.accessed;
                this.visible = true;
            }
            MouseArea { anchors.fill: parent }
            ColumnLayout {
                anchors.fill: parent
                Text {
                    id: filePath
                    text: ""
                    Layout.alignment: Qt.AlignCenter
                }
                Text {
                    id: fileUrl
                    text: ""
                    Layout.alignment: Qt.AlignCenter
                }
                Text {
                    id: fileSize
                    text: ""
                    Layout.alignment: Qt.AlignCenter
                }
                Text {
                    id: fileModified
                    text: ""
                    Layout.alignment: Qt.AlignCenter
                }
                Text {
                    id: fileAccessed
                    text: ""
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }
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
                if(fileInfo.visible){
                    fileInfo.visible = false;
                    fileList.visible  = true;
                    return;
                }
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
        Rectangle {
            color: "white"
            x: parseInt(pathInput.x + pathInput.leftPadding)
            y: parseInt(pathInput.y + pathInput.height - pathInput.bottomPadding)
            width: parseInt(pathInput.width - pathInput.leftPadding - pathInput.rightPadding)
            height: 1
        }
    }
    Keyboard { id: keyboard; visible: false }
}
