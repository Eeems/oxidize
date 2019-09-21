import QtQuick 2.9
import KeyboardHandler 1.0
import "."


Item {
    id: root
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    function shift(state){
        lshift.setState(state);
        rshift.setState(state);
        for(var i = 0; i < rows.children.length; i++){
            var row = rows.children[i];
            for(var y = 0; y < row.children.length; y++){
                row.children[y].shift = state;
            }
        }
    }
    function ctrl(state){
        lctrl.setState(state);
        rctrl.setState(state);
        for(var i = 0; i < rows.children.length; i++){
            var row = rows.children[i];
            for(var y = 0; y < row.children.length; y++){
                row.children[y].ctrl = state;
            }
        }
    }
    function alt(state){
        lalt.setState(state);
        ralt.setState(state);
        for(var i = 0; i < rows.children.length; i++){
            var row = rows.children[i];
            for(var y = 0; y < row.children.length; y++){
                row.children[y].alt = state;
            }
        }
    }

    width: parent.width
    height: 480
    MouseArea { anchors.fill: root }
    Rectangle {
        color: "black"
        anchors.fill: root
        Column {
            id: rows
            anchors.centerIn: parent
            spacing: 4
            Row {
                spacing: parent.spacing
                anchors.horizontalCenter: parent.horizontalCenter
                KeyboardKey { text: "`"; shifttext: "~" }
                KeyboardKey { text: "1"; shifttext: "!" }
                KeyboardKey { text: "2"; shifttext: "@" }
                KeyboardKey { text: "3"; shifttext: "#" }
                KeyboardKey { text: "4"; shifttext: "$" }
                KeyboardKey { text: "5"; shifttext: "%" }
                KeyboardKey { text: "6"; shifttext: "^" }
                KeyboardKey { text: "7"; shifttext: "&" }
                KeyboardKey { text: "8"; shifttext: "*" }
                KeyboardKey { text: "9"; shifttext: ")" }
                KeyboardKey { text: "0"; shifttext: "(" }
                KeyboardKey { text: "-"; shifttext: "_" }
                KeyboardKey { text: "="; shifttext: "+" }
                KeyboardKey { text: "Backspace"; size: 2 }
            }
            Row {
                spacing: parent.spacing
                anchors.horizontalCenter: parent.horizontalCenter
                KeyboardKey { text: "Tab"; size: 1 }
                KeyboardKey { text: "q"; shifttext: "Q" }
                KeyboardKey { text: "w"; shifttext: "W" }
                KeyboardKey { text: "e"; shifttext: "E" }
                KeyboardKey { text: "r"; shifttext: "R" }
                KeyboardKey { text: "t"; shifttext: "T" }
                KeyboardKey { text: "y"; shifttext: "Y" }
                KeyboardKey { text: "u"; shifttext: "U" }
                KeyboardKey { text: "i"; shifttext: "I" }
                KeyboardKey { text: "o"; shifttext: "O" }
                KeyboardKey { text: "p"; shifttext: "P" }
                KeyboardKey { text: "["; shifttext: "{" }
                KeyboardKey { text: "]"; shifttext: "}" }
                KeyboardKey { text: "\\"; shifttext: "|" }
            }
            Row {
                spacing: parent.spacing
                anchors.horizontalCenter: parent.horizontalCenter
                KeyboardKey { text: "Caps"; size: 2 }
                KeyboardKey { text: "a"; shifttext: "A" }
                KeyboardKey { text: "s"; shifttext: "S" }
                KeyboardKey { text: "d"; shifttext: "D" }
                KeyboardKey { text: "f"; shifttext: "F" }
                KeyboardKey { text: "g"; shifttext: "G" }
                KeyboardKey { text: "h"; shifttext: "H" }
                KeyboardKey { text: "j"; shifttext: "J" }
                KeyboardKey { text: "k"; shifttext: "K" }
                KeyboardKey { text: "l"; shifttext: "L" }
                KeyboardKey { text: ";"; shifttext: ":" }
                KeyboardKey { text: "'"; shifttext: "\"" }
                KeyboardKey { text: "Enter"; size: 2 }
            }
            Row {
                spacing: parent.spacing
                anchors.horizontalCenter: parent.horizontalCenter
                KeyboardKey { id: lshift; text: "Shift"; size: 2; onClick: root.shift(this.state()); toggle: true }
                KeyboardKey { text: "z"; shifttext: "Z" }
                KeyboardKey { text: "x"; shifttext: "X" }
                KeyboardKey { text: "c"; shifttext: "C" }
                KeyboardKey { text: "v"; shifttext: "V" }
                KeyboardKey { text: "b"; shifttext: "B" }
                KeyboardKey { text: "n"; shifttext: "N" }
                KeyboardKey { text: "m"; shifttext: "M" }
                KeyboardKey { text: ","; shifttext: "<" }
                KeyboardKey { text: "."; shifttext: ">" }
                KeyboardKey { text: "/"; shifttext: "?" }
                KeyboardKey { id: rshift; text: "Shift"; size: 2; onClick: root.shift(this.state()); toggle: true }
            }
            Row {
                spacing: parent.spacing
                anchors.horizontalCenter: parent.horizontalCenter
                KeyboardKey { id: lctrl; text: "Ctrl"; onClick: root.ctrl(this.state()); toggle: true }
                KeyboardKey { id: lalt; text: "Alt"; onClick: root.alt(this.state()); toggle: true }
                KeyboardKey { text: "Space"; size: 6 }
                KeyboardKey { id: ralt; text: "Alt"; onClick: root.alt(this.state()); toggle: true }
                KeyboardKey { text: "Menu"; fontsize: 6 }
                KeyboardKey { id: rctrl; text: "Ctrl"; onClick: root.ctrl(this.state()); toggle: true }
            }
        }
    }
    KeyboardKey {
        anchors.bottom: root.bottom
        anchors.left: root.left
        text: "\\/";
        onClick: {
            root.visible = false;
            root.focus = true;
        }
    }
    KeyboardHandler { objectName: "keyboard"; id: handler }
}
