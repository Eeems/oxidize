import QtQuick 2.0
import Qt.labs.folderlistmodel 1.0

Item {
    id: root
    clip: true
    property string cwd: "/"
    property string sortBy: "Name"
    property bool showOnlyReadable: false
    property bool showDirs: true
    property bool showDotAndDotDot: false
    property var filter: [ "*" ]
    property bool showScrollbar: true

    signal fileClick(File file)
    signal scrollStart(Item list)
    signal scrollEnd(Item list)
    signal touchStart(Item list)
    signal touchEnd(Item list)
    onCwdChanged: {
        if(root.showScrollbar){
            scrollbar.visible = false;
            scrollbarClear.visible = true;
            timer.start();
        }
    }
    ListView {
        id: fileList
        visible: fileModel.count > 0
        anchors.fill: root
        snapMode: ListView.SnapOneItem
        maximumFlickVelocity: 0
        boundsBehavior: Flickable.StopAtBounds
        FolderListModel {
            id: fileModel
            folder: "file:" + root.cwd
            nameFilters: root.filter
            showOnlyReadable: root.showOnlyReadable
            showDirs: root.showDirs
            showDotAndDotDot: root.showDotAndDotDot
            sortField: root.sortBy
        }
        Component {
            id: fileDelegate
            File {
                path: filePath
                name: fileName
                url: fileURL
                size: fileSize
                modified: fileModified
                accessed: fileAccessed
                isDir: fileIsDir
                onClick: root.fileClick(this)
                onTouchStart: root.touchStart(root)
                onTouchEnd: root.touchEnd(root)
                borderwidth: 0
            }
        }
        model: fileModel
        delegate: fileDelegate
        onMovementStarted: root.scrollStart(root)
        onMovementEnded: root.scrollEnd(root)
    }
    Rectangle {
       id: scrollbarClear
       width: 10
       radius: 10
       color: "white"
       visible: false
       anchors.right: fileList.right
       height: fileList.height
   }
    Rectangle {
       id: scrollbar
       width: 10
       radius: 5
       color: "black"
       visible: root.showScrollbar
       anchors.right: fileList.right
       y: fileList.visibleArea.yPosition * fileList.height
       height: fileList.visibleArea.heightRatio * fileList.height
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
   Timer {
       id: timer
       interval: 100
       repeat: false
       onTriggered: {
           scrollbar.visible = true;
           scrollbarClear.visible = false;
       }
   }
}
