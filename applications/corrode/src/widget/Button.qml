import QtQuick 2.0

Item {
    id: root
    property string text: ""

    property string color: "black"
    property string backgroundColor: "white"
    property string borderColor: "black"
    property string selectedColor: "white"
    property string selectedBackgroundColor: "black"
    property string selectedborderColor: "white"
    property int transitionTime: 300
    property int fontsize: 8
    property bool toggle: false
    signal click()
    signal hold()
    signal release()


    property bool isSelected: false
    property bool isHeld: false

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
                if(root.toggle){
                    root.isSelected = !root.isSelected;
                }else{
                    root.isSelected = true;
                    timer.start();
                }
                console.log("click (" + this + ") " +  root.isSelected);
                root.click()
            }
            onPressAndHold: {
                root.isSelected = true;
                root.isHeld = true;
                console.log("hold " + this);
                root.hold()
            }
            onReleased: {
                if(root.isHeld){
                    root.isSelected = false;
                }
                console.log("release (" + this + ")");
                root.release()
            }
        }
    }
    Text {
        id: label
        color: root.isSelected ? root.selectedColor : root.color
        text: root.text
        anchors.centerIn: root
        font.pointSize: fontsize
    }
    Timer {
        id: timer
        interval: root.transitionTime
        repeat: false
        onTriggered: {
            root.isSelected = false;
            console.log("click clear (" + this + ") " +  root.isSelected);
        }
    }
}
