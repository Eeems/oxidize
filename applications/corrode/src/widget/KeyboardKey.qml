import QtQuick 2.0
import "."

Item {
    id: root
    property string text: ""
    property string shifttext: text
    property string alttext: text
    property string ctrltext: text
    property string shiftalttext: shifttext
    property string shiftctrltext: shifttext
    property string altctrltext: alttext
    property string shiftaltctrltext: shiftalttext
    property int size: 1
    property int basesize: 90
    property int fontsize: 8
    signal click()
    property bool shift: false
    property bool alt: false
    property bool ctrl: false
    property bool toggle: false
    function state(){
        return button.isSelected;
    }
    function setState(state){
        button.isSelected = state;
    }

    width: size * basesize
    height: basesize
    function getText(){
        if(shift && alt && ctrl){
            return shiftaltctrltext;
        }
        if(shift && alt){
            return shiftalttext;
        }
        if(shift && ctrl){
            return shiftctrltext;
        }
        if(alt && ctrl){
            return altctrltext;
        }
        if(shift){
            return shifttext;
        }
        if(alt){
            return alttext;
        }
        if(ctrl){
            return ctrltext;
        }
        return text;
    }
    function update(){
        button.text = root.getText()
    }

    Button {
        id: button
        text: root.getText()
        anchors.fill: parent
        fontsize: root.fontsize
        toggle: root.toggle
        onClick: {
            root.click()
        }
    }
}
