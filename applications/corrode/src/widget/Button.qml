import QtQuick 2.0

Item {
    id: root
    property string text: ""

    property string color: "black"
    property string backgroundColor: "#22000000"
    property string borderColor: "#22000000"
    property string selectedColor: "black"
    property string selectedBackgroundColor: "white"
    property string selectedborderColor: "black"
    signal click()
    signal hold()
    signal release()


    property bool isSelected: false

    Rectangle {
        width: root.width
        height: root.height
        anchors.centerIn: root
        color: root.isSelected ? root.selectedBackgroundColor : root.backgroundColor
        border.color: root.isSelected ? root.selectedborderColor : root.borderColor
        border.width: 5
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                console.log("click " + this);
                root.isSelected = true;
                timer.start();
                root.click()
            }
            onPressAndHold: {
                console.log("hold " + this);
                root.isSelected = true;
                root.hold()
            }
            onReleased: {
                root.isSelected = false;
                root.release()
            }
        }
    }
    Text {
        id: label
        color: root.isSelected ? root.selectedColor : root.color
        text: root.text
        anchors.centerIn: root
    }
    Timer {
        id: timer
        interval: 300
        repeat: false
        onTriggered: {
            root.isSelected = false;
        }
    }
}
