import QtMultimedia 5.5
import QtQuick 2.5

Item {
    width: 960
    height: 160

    Video {
        anchors.fill: parent
        autoPlay: true
        fillMode: VideoOutput.Stretch
        source: "test.avi"
    }

    Component.onCompleted: {
        console.log("Loaded");
    }
}
