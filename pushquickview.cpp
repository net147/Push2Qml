#include "pushquickview.h"
#include <QQmlEngine>
#include "pushdisplay.h"
#include "RtMidi.h"

class PushDisplayEvents : public QObject
{
    Q_OBJECT

public:
    PushDisplayEvents() { }
    ~PushDisplayEvents() { }

signals:
    void clearLine(int index);
    void writeLine(int index, const QString &text);
};

static PushDisplayEvents *pushDisplayEvents = 0;

static QObject *pushDisplayEventsProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    if (!pushDisplayEvents)
        pushDisplayEvents = new PushDisplayEvents;

    return pushDisplayEvents;
}

class PushQuickViewPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(PushQuickView)

public:
    PushQuickViewPrivate(PushQuickView *q_ptr);
    ~PushQuickViewPrivate();
    static void push1MidiOutCallback(double timeStamp, std::vector<uchar> *message, void *userData);
    static void push2MidiInCallback(double timeStamp, std::vector<uchar> *message, void *userData);

public slots:
    void sceneRendered();

public:
    PushDisplay display;

    QScopedPointer<RtMidiIn> push1MidiOut;
    QScopedPointer<RtMidiOut> push2MidiOut;

    QScopedPointer<RtMidiIn> push2MidiIn;
    QScopedPointer<RtMidiOut> push1MidiIn;

    PushQuickView * const q_ptr;
};

PushQuickViewPrivate::PushQuickViewPrivate(PushQuickView *q_ptr) :
    q_ptr(q_ptr)
{
    qmlRegisterSingletonType<PushDisplayEvents>("Push2Qml.Emulation", 1, 0, "PushDisplayEvents",
                                                &pushDisplayEventsProvider);
    connect(q_ptr, SIGNAL(sceneRendered()), SLOT(sceneRendered()));

    push1MidiOut.reset(new RtMidiIn);
    push1MidiOut->ignoreTypes(false, false, false);
    push1MidiOut->setCallback(&push1MidiOutCallback, this);

    for (int i = 0, portCount = push1MidiOut->getPortCount(); i < portCount; ++i) {
        if (QString::fromStdString(push1MidiOut->getPortName(i)) == "MIDIOUT2 (Ableton Push)") {
            push1MidiOut->openPort(i);
            break;
        }
    }

    push2MidiOut.reset(new RtMidiOut);

    for (int i = 0, portCount = push2MidiOut->getPortCount(); i < portCount; ++i) {
        if (QString::fromStdString(push2MidiOut->getPortName(i)) == "Ableton Push 2") {
            push2MidiOut->openPort(i);
            break;
        }
    }

    push2MidiIn.reset(new RtMidiIn);
    push2MidiIn->ignoreTypes(false, false, false);
    push2MidiIn->setCallback(&push2MidiInCallback, this);

    for (int i = 0, portCount = push2MidiIn->getPortCount(); i < portCount; ++i) {
        if (QString::fromStdString(push2MidiIn->getPortName(i)) == "Ableton Push 2") {
            push2MidiIn->openPort(i);
            break;
        }
    }

    push1MidiIn.reset(new RtMidiOut);

    for (int i = 0, portCount = push1MidiIn->getPortCount(); i < portCount; ++i) {
        if (QString::fromStdString(push1MidiIn->getPortName(i)) == "MIDIIN2 (Ableton Push)") {
            push1MidiIn->openPort(i);
            break;
        }
    }

    if (push1MidiOut->isPortOpen() && push2MidiOut->isPortOpen()
            && push2MidiIn->isPortOpen() && push1MidiIn->isPortOpen()) {
        qDebug("Intercepting SysEx from MIDI input \"MIDIOUT2 (Ableton Push)\"");
        qDebug("Forwarding MIDI input \"MIDIOUT2 (Ableton Push)\" to MIDI output \"Ableton Push 2\"");
        qDebug("Forwarding MIDI input \"Ableton Push 2\" to MIDI output \"MIDIIN2 (Ableton Push)\"");
        qDebug("Mapping Volume (CC 114) on Push 1 to Convert button (CC 35) on Push 2");
        qDebug("Mapping Pan & Send (CC 115) on Push 1 to Setup button (CC 30) on Push 2");
        qDebug("Mapping LED colors and behavior on Push 1 to Push 2");
        qDebug("Push 1 display emulation started");
    } else {
        push1MidiOut.reset();
        push2MidiOut.reset();
        push2MidiIn.reset();
        push1MidiIn.reset();
        qDebug("Push 1 display emulation MIDI loopback ports not detected: "
               "\"MIDIIN2 (Ableton Push)\" and \"MIDIOUT2 (Ableton Push)\"");
    }
}

PushQuickViewPrivate::~PushQuickViewPrivate()
{
}

void PushQuickViewPrivate::push1MidiOutCallback(double timeStamp, std::vector<uchar> *message, void *userData)
{
    Q_UNUSED(timeStamp);

    PushQuickViewPrivate *self = static_cast<PushQuickViewPrivate *>(userData);

    if (!self)
        return;

    QByteArray data(reinterpret_cast<const char *>(message->data()), static_cast<int>(message->size()));

    const int colorMap[] = {
        0,   119, 56,  120,  1,  127,  65,  66,
        38,  2,   67,   76,  5,    7,  77,  78,
        11,  126, 85,   81,  11, 126,  85,  86,
        11,  126, 85,   86,  43,  11,  85,  86,
        44,  44,  13,   92,  47,  48,  91,  94,
        15,  18,  99,   94,  18, 125, 107, 100,
        18,  125, 103, 108,  21,  21, 105, 108,
        23,  1,   65,  110, 127,  28,  29,  10,
        81,  89,  97,  125,  91, 125,  56, 119,
        127, 5,   5,   126,   9,  43,  48, 125,
        125, 20,  23,   69,   2,   8, 126, 126,
        11,  11,  44,   15,  16,  34,  20,  24,
        3,   6,   126,  29,  75,  85,  89, 100,
        101, 28,  27,   27,  28,   7,   8,   8,
        102, 5,   44,   50,  18, 118, 118, 120,
        27,  65,  8,    79,   6,  75,  28,  67
    };

    if (!data.startsWith(char(0xf0)) || !data.endsWith(char(0xf7))) {
        if (self->push2MidiOut) {
            int blinkSpeed = 0;

            // Map red Automation (CC 29), Record (CC 86) and Stop (CC 89) buttons with white LED (channel 1 only)
            // on Push 1 to Push 2
            if (data.size() == 3 && data.at(0) == char(0xb0)
                    && (data.at(1) == 29 || data.at(1) == 86 || data.at(1) == 89)) {
                int index = data.at(2);

                if (index == 0)
                    data[2] = 0; // Off
                else if (index >= 1 && index <= 3)
                    data[2] = colorMap[121]; // Red Dim
                else
                    data[2] = colorMap[120]; // Red

                if (index >= 1 && index <= 6)
                    blinkSpeed = (index - 1) % 3;
            // Map green Play button (CC 85) with white LED (channel 1 only) on Push 1 to Push 2
            } else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 85) {
                int index = data.at(2);

                if (index == 0)
                    data[2] = 0; // Off
                else if (index >= 1 && index <= 3)
                    data[2] = colorMap[123]; // Green Dim
                else
                    data[2] = colorMap[122]; // Green

                if (index >= 1 && index <= 6)
                    blinkSpeed = (index - 1) % 3;
            // Map blue Solo button (CC 61) with white LED (channel 1 only) on Push 1 to Push 2
            } else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 61) {
                int index = data.at(2);

                if (index == 0)
                    data[2] = 0; // Off
                else if (index >= 1 && index <= 3)
                    data[2] = colorMap[40]; // Blue Dim
                else
                    data[2] = colorMap[45]; // Blue

                if (index >= 1 && index <= 6)
                    blinkSpeed = (index - 1) % 3;
            // Map amber Mute buttion (CC 60) with white LED (channel 1 only) on Push 1 to Push 2
            } else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 60) {
                int index = data.at(2);

                if (index == 0)
                    data[2] = 0; // Off
                else if (index >= 1 && index <= 3)
                    data[2] = colorMap[127]; // Amber Dim
                else
                    data[2] = colorMap[126]; // Amber

                if (index >= 1 && index <= 6)
                    blinkSpeed = (index - 1) % 3;
            // Map white LEDs (channel 1 only) on Push 1 to Push 2
            } else if (data.size() == 3 && data.at(0) == char(0xb0)
                    && (
                        // CC 3
                        data.at(1) == 3
                        // CC 9
                        || data.at(1) == 9
                        // CC 28 to CC 29
                        || data.at(1) >= 28 && data.at(1) <= 29
                        // CC 44 to CC 63 (excluding CC 60)
                        || data.at(1) >= 44 && data.at(1) <= 63 && data.at(1) != 60
                        // CC 85 to CC 90
                        || data.at(1) >= 85 && data.at(1) <= 90
                        // CC 110 to CC 119
                        || data.at(1) >= 110 && data.at(1) <= 119
                        )
                    ) {
                int index = data.at(2);

                if (index == 0)
                    data[2] = 0; // Off
                else if (index >= 1 && index <= 3)
                    data[2] = 6; // Dim
                else
                    data[2] = 127; // On

                if (index >= 1 && index <= 6)
                    blinkSpeed = (index - 1) % 3;
            // Map RG (red/green bi-color) LEDs (channel 1 only) on Push 1 to Push 2
            } else if (data.size() == 3 && data.at(0) == char(0xb0)
                    && (
                        // First row of buttons under the display (CC 20 to CC 27)
                        data.at(1) >= 20 && data.at(1) <= 27
                        // Time resolution buttons to the right of the pads (CC 36 to CC 43)
                        || data.at(1) >= 36 && data.at(1) <= 43)
                    ) {
                int index = data.at(2);

                if (index == 0)
                    data[2] = colorMap[0]; // Black
                else if (index >= 1 && index <= 3)
                    data[2] = colorMap[121]; // Red Dim
                else if (index >= 4 && index <= 6)
                    data[2] = colorMap[120]; // Red
                else if (index >= 7 && index <= 9)
                    data[2] = colorMap[127]; // Amber Dim
                else if (index >= 10 && index <= 12)
                    data[2] = colorMap[126]; // Amber
                else if (index >= 13 && index <= 15)
                    data[2] = colorMap[125]; // Yellow Dim
                else if (index >= 16 && index <= 18)
                    data[2] = colorMap[124]; // Yellow
                else if (index >= 19 && index <= 21)
                    data[2] = colorMap[123]; // Green Dim
                else if (index >= 22 && index <= 24)
                    data[2] = colorMap[122]; // Green
                else
                    data[2] = colorMap[122]; // Green

                if (index >= 1 && index <= 24)
                    blinkSpeed = (index - 1) % 3;
            // Map RGB LEDs on Push 1 to Push 2
            } else if (data.size() == 3
                     && (
                         // Second row of buttons under the display (CC 102 to CC 109)
                         data.at(0) >= char(0xb0) && data.at(0) <= char(0xbf) && data.at(1) >= 102 && data.at(1) <= 109
                         // 8x8 pads (note 36 to note 99)
                         || data.at(0) >= char(0x90) && data.at(0) <= char(0x9f) && data.at(1) >= 36 && data.at(1) <= 99
                         )
                     )
                data[2] = colorMap[data.at(2)];

            // Map Volume (CC 114) on Push 1 to Convert button (CC 35) on Push 2
            if (data.size() == 3 && data.at(0) >= char(0xb0) && data.at(0) <= char(0xbf)
                    && data.at(1) == 114)
                data[1] = char(35);
            // Map Pan & Send (CC 115) on Push 1 to Setup button (CC 30) on Push 2
            else if (data.size() == 3 && data.at(0) >= char(0xb0) && data.at(0) <= char(0xbf)
                     && data.at(1) == 115)
                data[1] = char(30);
            // Map CC 20-27 on Push 1 to CC 102-109 on Push 2
            else if (data.size() == 3 && data.at(0) >= char(0xb0) && data.at(0) <= char(0xbf)
                     && data.at(1) >= 20 && data.at(1) <= 27)
                data[1] = data.at(1) + (102 - 20);
            // Map CC 102-109 on Push 1 to CC 20-27 on Push 2
            else if (data.size() == 3 && data.at(0) >= char(0xb0) && data.at(0) <= char(0xbf)
                     && data.at(1) >= 102 && data.at(1) <= 109)
                data[1] = data.at(1) - (102 - 20);

            for (int i = 0; i < (blinkSpeed ? 2 : 1); ++i){
                if (i > 0) {
                    switch (blinkSpeed) {
                    case 1: // 0.5 seconds on, 0.5 seconds off, ...
                        data[0] = data.at(0) + 15;
                        data[2] = 0;
                        break;
                    case 2: // 0.25 seconds on, 0.25 seconds off, ...
                        data[0] = data.at(0) + 14;
                        data[2] = 0;
                        break;
                    default:
                        break;
                    }
                }

                std::vector<uchar> outputMessage(data.constBegin(), data.constEnd());

                self->push2MidiOut->sendMessage(&outputMessage);
            }
        }
    }

    if (!pushDisplayEvents)
        return;

    if (data == QByteArray::fromHex("f0477f151c0000f7")) {
        emit pushDisplayEvents->clearLine(0);
    } else if (data == QByteArray::fromHex("f0477f151d0000f7")) {
        emit pushDisplayEvents->clearLine(1);
    } else if (data == QByteArray::fromHex("f0477f151e0000f7")) {
        emit pushDisplayEvents->clearLine(2);
    } else if (data == QByteArray::fromHex("f0477f151f0000f7")) {
        emit pushDisplayEvents->clearLine(3);
    } else if (data.size() == 77 && data.startsWith(QByteArray::fromHex("f0477f15"))
               && uchar(data.at(4)) >= 24 && uchar(data.at(4)) <= 27
               && data.mid(5, 3) == QByteArray::fromHex("004500")
               && data.endsWith(char(0xf7))) {
        int index = uchar(data.at(4)) - 24;
        QByteArray lineData = data.mid(8, 68);
        QString text;

        text.resize(68);

        for (int i = 0; i < lineData.size(); ++i)
            text[i] = QChar(uchar(lineData[i]));

        emit pushDisplayEvents->writeLine(index, text);
    }
}

void PushQuickViewPrivate::push2MidiInCallback(double timeStamp, std::vector<uchar> *message, void *userData)
{
    Q_UNUSED(timeStamp);

    PushQuickViewPrivate *self = static_cast<PushQuickViewPrivate *>(userData);

    if (!self)
        return;

    if (self->push1MidiIn) {
        QByteArray data(reinterpret_cast<const char *>(message->data()), static_cast<int>(message->size()));

        // Map Convert button (CC 35) on Push 2 to Volume (CC 114) on Push 1
        if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 35)
            data[1] = char(114);
        // Map Setup button (CC 30) on Push 2 to Pan & Send (CC 115) on Push 1
        else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 30)
            data[1] = char(115);
        // Map CC 102-109 on Push 2 to CC 20-27 on Push 1
        else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) >= 102 && data.at(1) <= 109)
            data[1] = data.at(1) - (102 - 20);
        // Map CC 20-27 on Push 2 to CC 102-109 on Push 1
        else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) >= 20 && data.at(1) <= 27)
            data[1] = data.at(1) + (102 - 20);

        std::vector<uchar> outputMessage(data.constBegin(), data.constEnd());

        self->push1MidiIn->sendMessage(&outputMessage);
    }
}

void PushQuickViewPrivate::sceneRendered()
{
    Q_Q(PushQuickView);
    display.drawImage(q->grab());
}

PushQuickView::PushQuickView(const QUrl &url) :
    d_ptr(new PushQuickViewPrivate(this))
{
    Q_D(PushQuickView);

    if (!isOpen())
        return;

    setGeometry(QRect(QPoint(0, 0), d->display.size()));
    setSource(url);
}

PushQuickView::~PushQuickView()
{
}

bool PushQuickView::isOpen() const
{
    Q_D(const PushQuickView);
    return d->display.isOpen();
}

bool PushQuickView::dithering() const
{
    Q_D(const PushQuickView);
    return d->display.dithering();
}

void PushQuickView::setDithering(bool value)
{
    Q_D(PushQuickView);
    if (d->display.dithering() != value) {
        d->display.setDithering(value);
        emit ditheringChanged();
    }
}

#include "pushquickview.moc"
