import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.folderlistmodel 2.2

Item {
    id: root
    clip: true
    property string cwd: "/"
    property string sortBy: "Name"
    property bool sortReversed: false
    property bool showOnlyReadable: false
    property bool showDirs: true
    property bool showDirsFirst: true
    property bool showDotAndDotDot: false
    property var filter: [ "*" ]
    property bool showScrollbar: true

    signal fileClick(File file)
    signal fileLongPress(File file)
    signal scrollStart(Item list)
    signal scrollEnd(Item list)
    signal touchStart(Item list)
    signal touchEnd(Item list)
    Item {
        id: fileListHeader
        visible: fileModel.count > 0
        width: root.width
        height: 50
        anchors.top: root.top
        RowLayout {
            anchors.fill: parent
            Text {
                text: "Name"
                color: "black"
                font.pointSize: 8
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                leftPadding: 10
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(root.sortBy != "Name"){
                            root.sortBy = "Name";
                            root.sortReversed = false;
                        }else{
                            root.sortReversed = !root.sortReversed;
                        }
                    }
                }
            }
            Text {
                text: "Size"
                color: "black"
                font.pointSize: 8
                Layout.alignment: Qt.AlignRight
                rightPadding: 26
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(root.sortBy != "Size"){
                            root.sortBy = "Size";
                            root.sortReversed = false;
                        }else{
                            root.sortReversed = !root.sortReversed;
                        }
                    }
                }
            }
        }
        Rectangle {
            color: "grey"
            width: parent.width
            anchors.top: parent.bottom
            height: 2
        }
    }
    ListView {
        id: fileList
        clip: true
        visible: fileModel.count > 0
        width: root.width
        height: root.height - fileListHeader.height
        anchors.top: fileListHeader.bottom
        snapMode: ListView.SnapOneItem
        maximumFlickVelocity: 0
        boundsBehavior: Flickable.StopAtBounds
        FolderListModel {
            id: fileModel
            folder: "file:" + root.cwd
            nameFilters: root.filter
            showOnlyReadable: root.showOnlyReadable
            showDirs: root.showDirs
            showDirsFirst: root.showDirsFirst
            showDotAndDotDot: root.showDotAndDotDot
            sortField: root.sortBy
            sortReversed: root.sortReversed
        }
        Component {
            id: fileDelegate
            File {
                path: filePath
                name: fileName
                url: fileURL
                width: fileList.width - scrollbar.width
                size: fileSize
                modified: fileModified
                accessed: fileAccessed
                isDir: fileIsDir
                onClick: root.fileClick(this)
                onLongPress: root.fileLongPress(this)
            }
        }
        model: fileModel
        delegate: fileDelegate
        onMovementStarted: root.scrollStart(root)
        onMovementEnded: root.scrollEnd(root)
        ScrollBar.vertical: ScrollBar {
            id: scrollbar
            visible: root.showScrollbar
            enabled: this.visible
            snapMode: ScrollBar.SnapAlways
            policy: ScrollBar.AlwaysOn
            contentItem: Rectangle {
                color: "white"
                implicitWidth: 6
                implicitHeight: 100
                radius: width / 2
            }
            background: Rectangle {
                color: "black"
                implicitWidth: 6
                implicitHeight: 100
                radius: width / 2
            }
        }
    }
    Text {
        anchors.centerIn: root
        visible: fileModel.count === 0
        text: "No Files..."
        color: "grey"
        MultiPointTouchArea {
            enabled: parent.visible
            anchors.fill: parent
            onPressed: root.touchStart(root)
            onReleased: root.touchEnd(root)
        }
   }
   Rectangle {
       id: loadingIndicatorCont
       anchors.centerIn: parent
       width: 100
       height: 100
       visible: false
       Text {
           id: loadingIndicator
           anchors.centerIn: parent
           visible: false
           text: "Loading..."
           color: "white"
       }
       state: "loading"
       transitions: [
           Transition {
                from: "loading"
                to: "loaded"
                SequentialAnimation {
                    PropertyAction { target: loadingIndicatorCont; property: "visible"; value: true }
                    PropertyAction { target: loadingIndicator; property: "visible"; value: true }
                    PropertyAction { target: scrollbar; property: "visible"; value: false }
                    PauseAnimation { duration: 50 }
                    PropertyAction { target: loadingIndicatorCont; property: "visible"; value: false }
                    PropertyAction { target: loadingIndicator; property: "visible"; value: false }
                    PropertyAction { target: scrollbar; property: "visible"; value: root.showScrollbar }
                }
           },
           Transition {
                from: "loaded"
                to: "loading"
                SequentialAnimation {
                    PropertyAction { target: loadingIndicatorCont; property: "visible"; value: false }
                    PropertyAction { target: loadingIndicator; property: "visible"; value: false }
                    PropertyAction { target: scrollbar; property: "visible"; value: root.showScrollbar }
                    PauseAnimation { duration: 100 }
                    PropertyAction { target: loadingIndicatorCont; property: "visible"; value: true }
                    PropertyAction { target: loadingIndicator; property: "visible"; value: true }
                    PropertyAction { target: scrollbar; property: "visible"; value: false }
                }
           }
       ]
       states: [
           State { name: 'loading'; when: fileModel.status === FolderListModel.Loading },
           State { name: 'loaded'; when: fileModel.status === FolderListModel.Ready }
       ]
   }
}
