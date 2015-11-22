import QtQuick 2.5
import QtGraphicalEffects 1.0
import Push2Qml.Emulation 1.0

Rectangle {
    property double lcdZoom: 2.6
    color: "#200d0f"
    width: 960
    height: 160

    Connections {
        target: PushDisplayEvents
        onClearLine: {
            switch (index) {
                case 0:
                    display.line0 = "";
                    break;
                case 1:
                    display.line1 = "";
                    break;
                case 2:
                    display.line2 = "";
                    break;
                case 3:
                    display.line3 = "";
                    break;
            }
        }
        onWriteLine: {
            switch (index) {
                case 0:
                    display.line0 = text;
                    break;
                case 1:
                    display.line1 = text;
                    break;
                case 2:
                    display.line2 = text;
                    break;
                case 3:
                    display.line3 = text;
                    break;
            }
        }
    }

    Component {
        id: displayCell

        Row {
            property alias characterCode: character.currentFrame

            Item {
                width: 5 * lcdZoom
                height: 8 * lcdZoom

                Rectangle {
                    anchors.fill: parent
                    color: "#4a2713"
                }

                ImageSprite {
                    id: character
                    anchors.fill: parent
                    frameCount: 128
                    frameWidth: 5
                    frameHeight: 8
                    source: "PushCharset.png"
                }
            }

            Item {
                width: 1
                height: 1
            }
        }
    }

    Component {
        id: displaySegment

        Row {
            property string displayText

            Repeater {
                model: 17
                delegate: Component {
                    Loader {
                        id: loader
                        sourceComponent: displayCell

                        Binding {
                            target: loader.item
                            property: "characterCode"
                            value: index < displayText.length ? displayText.charCodeAt(index) : 32
                        }
                    }
                }
            }
        }
    }

    Column {
        id: display
        property string line0
        property string line1
        property string line2
        property string line3
        spacing: 3
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter

        Repeater {
            model: 4

            Row {
                property string displayText: [display.line0, display.line1, display.line2, display.line3][index]
                spacing: 0

                Loader {
                    id: segment1
                    sourceComponent: displaySegment

                    Binding {
                        target: segment1.item
                        property: "displayText"
                        value: displayText.substr(17 * 0, 17)
                    }
                }

                Item {
                    width: 2
                    height: 1
                }

                Loader {
                    id: segment2
                    sourceComponent: displaySegment

                    Binding {
                        target: segment2.item
                        property: "displayText"
                        value: displayText.substr(17 * 1, 17)
                    }
                }

                Item {
                    width: 3
                    height: 1
                }

                Loader {
                    id: segment3
                    sourceComponent: displaySegment

                    Binding {
                        target: segment3.item
                        property: "displayText"
                        value: displayText.substr(17 * 2, 17)
                    }
                }

                Item {
                    width: 2
                    height: 1
                }

                Loader {
                    id: segment4
                    sourceComponent: displaySegment

                    Binding {
                        target: segment4.item
                        property: "displayText"
                        value: displayText.substr(17 * 3, 17)
                    }
                }
            }
        }
    }
}
