import QtQuick 2.5

Item {
    id: root
    anchors.fill: parent
    property int frameCount: 1
    property int frameWidth: sourceImage.width
    property int frameHeight: sourceImage.height
    property int currentFrame: 0
    property alias source: sourceImage.source
    property alias smooth: sourceImage.smooth

    Item {
        id: sprite
        width: frameWidth
        height: frameHeight
        clip: true
        readonly property int framesPerLine: sourceImage.width / frameWidth

        Image {
            id: sourceImage
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: -(root.currentFrame % sprite.framesPerLine) * root.frameWidth
            anchors.topMargin: -Math.floor(root.currentFrame / sprite.framesPerLine) * root.frameHeight
            smooth: false
        }

        transform: Scale { xScale: root.width / sprite.width; yScale: root.height / sprite.height }
    }
}
