import QtQuick 2.9
import QtQuick.Layouts 1.3

Item {
    id: root
    height: 70
    width: 100
    clip: true
    property string path: ""
    property string name: ""
    property string url: ""
    property int size: 0
    property string humanSize: {
        var bytes = this.size;
        if(bytes === 0) {
            return "0.00 B";
        }
        var e = Math.floor(Math.log(bytes) / Math.log(1024));
        return (bytes/Math.pow(1024, e)).toFixed(2)+' '+' KMGTP'.charAt(e)+'B';
    }

    property date modified: new Date()
    property date accessed: new Date()
    property bool isDir: false
    property bool isClicked: false
    property bool isFile: !root.isDir
    property string text: root.name + (root.isDir ? "/" : "")
    property bool held: false
    signal click(Item file)
    signal longPress(Item file)
    MouseArea {
        anchors.fill: root
        onPressed: timer.start()
        onClicked: {
            root.isClicked = true;
            timer.stop();
            timer2.start();
        }
        onPressAndHold: root.held = true
        onReleased: {
            timer.stop();
            if(root.isClicked){
                root.isClicked = false;
                if(root.held){
                    root.held = false;
                    root.longPress(root);
                }
            }
        }
        onExited: {
            timer.stop();
            root.held = false;
            root.isClicked = false;
        }
        onCanceled: {
            timer.stop();
            root.held = false;
            root.isClicked = false;
        }
    }
    Timer { id: timer; interval: 100; onTriggered: root.isClicked = true }
    Timer { id: timer2; interval: 10; onTriggered: { root.isClicked = false; root.click(root); }}
    RowLayout {
        anchors.fill: root
        Text {
            id: label
            text: root.text
            color: "black"
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            leftPadding: 10
        }
        Text {
            text: root.humanSize
            color: label.color
            Layout.alignment: Qt.AlignRight
            rightPadding: 10
        }
    }
    states: [
        State { name: "resting"; when: !root.isClicked },
        State { name: "pressed"; when: root.isClicked }
    ]
    transitions: [
        Transition {
            from: "resting"
            to: "pressed"
            SequentialAnimation {
                PropertyAction { target: label; property: "color"; value: "black" }
                PauseAnimation { duration: 100 }
                PropertyAction { target: label; property: "color"; value: "white" }
            }
        },
        Transition {
            from: "pressed"
            to: "resting"
            SequentialAnimation {
                PropertyAction { target: label; property: "color"; value: "white" }
                PauseAnimation { duration: 100 }
                PropertyAction { target: label; property: "color"; value: "black" }
            }
        }
    ]
}
