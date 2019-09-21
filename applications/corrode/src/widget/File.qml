import QtQuick 2.0

Item {
    id: root
    height: 100
    clip: true
    width: screenGeometry.width / (parent.columns || 1)
    property string path: ""
    property string name: ""
    property string url: ""
    property int size: 0
    property date modified: new Date()
    property date accessed: new Date()
    property bool isDir: false
    property bool isClicked: false
    property bool isFile: !root.isDir
    property int borderwidth: 3
    property string text: root.name + (root.isDir ? "/" : "")
    signal click(Item file)
    signal touchStart(Item file)
    signal touchEnd(Item file)
    MultiPointTouchArea {
        anchors.fill: root
        onPressed: root.touchStart(root)
        onReleased: {
            root.touchEnd(root)
            root.isClicked = true;
            timer.start();
        }
    }
    Rectangle {
        anchors.fill: root
        border.color: "black"
        border.width: root.borderwidth
        color: root.isClicked ? "black" : "transparent"
    }
    Text {
        anchors.centerIn: root
        text: root.text
    }
    Timer {
        id: timer
        interval: 300
        repeat: false
        onTriggered: {
            root.isClicked = false;
            root.click(root);
        }
    }
}
