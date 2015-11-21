import QtQuick 2.5

Item {
    width: 960
    height: 160

    Image {
        anchors.fill: parent
        source: "test.png"
    }

    Component.onCompleted: {
        console.log("Loaded");
    }
}
