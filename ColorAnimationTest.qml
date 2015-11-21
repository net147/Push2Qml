import QtQuick 2.5

Item {
    width: 960
    height: 160

    Rectangle {
        anchors.fill: parent
        color: "red"
        border.width: 1
        border.color: "white"

        Text {
            anchors.centerIn: parent
            font.pixelSize: 48
            text: "Hello world!"
            color: "black"

            SequentialAnimation on font.pixelSize {
                running: true
                loops: Animation.Infinite
                PropertyAnimation { duration: 1000; to: 64 }
                PropertyAnimation { duration: 1000; to: 48 }
            }
        }

        SequentialAnimation on color {
            running: true
            loops: Animation.Infinite
            ColorAnimation {
                from: "red"
                to: "green"
                duration: 1000
            }
            ColorAnimation {
                from: "green"
                to: "blue"
                duration: 1000
            }
            ColorAnimation {
                from: "blue"
                to: "green"
                duration: 1000
            }
            ColorAnimation {
                from: "green"
                to: "red"
                duration: 1000
            }
        }
    }

    Component.onCompleted: {
        console.log("Loaded");
    }
}
