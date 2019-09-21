import QtQuick 2.9
import "."

Item {
    id: root
    property string text: ""
    property string shifttext: text
    property string alttext: text
    property string ctrltext: text
    property string metatext: text
    property string shiftalttext: shifttext
    property string shiftctrltext: shifttext
    property string shiftmetatext: shifttext
    property string altctrltext: alttext
    property string altmetatext: alttext
    property string ctrlmetatext: alttext
    property string shiftaltctrltext: shiftalttext
    property string shiftaltmetatext: shiftalttext
    property string shiftctrlmetatext: shiftctrltext
    property string shiftaltctrlmetatext: shiftalttext
    property int key: 0
    property string value: null
    property int size: 1
    property int basesize: 90
    property int fontsize: 8
    signal click(Item item)
    property bool shift: false
    property bool alt: false
    property bool ctrl: false
    property bool meta: false
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
    Button {
        id: button
        text: root.getText()
        anchors.fill: parent
        fontsize: root.fontsize
        toggle: root.toggle
        color: "white"
        backgroundColor: "transparent"
        borderColor: "transparent"
        selectedColor: "black"
        selectedBackgroundColor: "white"
        selectedborderColor: "white"
        borderwidth: 3
        onClick: {
            if(root.click(root) !== false){
                var modifiers = Qt.NoModifier;
                if(root.shift){
                    modifiers = modifiers | Qt.ShiftModifier;
                }
                if(root.alt){
                    modifiers = modifiers | Qt.AltModifier;
                }
                if(root.ctrl){
                    modifiers = modifiers | Qt.ControlModifier;
                }
                if(root.meta){
                    modifiers = modifiers | Qt.MetaModifier;
                }
                if(root.key > 0){
                    handler.keyPress(root.key, modifiers, root.value);
                }else{
                    var text = root.getText();
                    if(text.length > 0){
                        handler.stringPress(text, modifiers, text);
                    }else{
                        handler.charPress(text, modifiers);
                    }
                }
                keyboard.click(root)
            }
        }
    }
}
