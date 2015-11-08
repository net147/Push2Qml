import QtQuick 2.5

Item {
    width: 1024
    height: 160

    Rectangle {
        anchors.fill: parent
        color: "red"

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
}
