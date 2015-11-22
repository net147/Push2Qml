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

    if (push1MidiOut->isPortOpen()) {
    } else {
        push1MidiOut.reset();
        return;
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

    QByteArray data = QByteArray::fromRawData(reinterpret_cast<const char *>(message->data()),
                                              static_cast<int>(message->size()));

    if (!data.startsWith(char(0xf0)) || !data.endsWith(char(0xf7))) {
        if (self->push2MidiOut) {
            // Map Volume (CC 114) on Push 1 to Convert button (CC 35) on Push 2
            if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 114)
                data[1] = char(35);
            // Map Pan & Send (CC 115) on Push 1 to Setup button (CC 30) on Push 2
            else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) == 115)
                data[1] = char(30);
            // Map CC 20-27 on Push 1 to CC 102-109 on Push 2
            else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) >= 20 && data.at(1) <= 27)
                data[1] = data.at(1) + (102 - 20);
            // Map CC 102-109 on Push 1 to CC 20-27 on Push 2
            else if (data.size() == 3 && data.at(0) == char(0xb0) && data.at(1) >= 102 && data.at(1) <= 109)
                data[1] = data.at(1) - (102 - 20);

            std::vector<uchar> outputMessage(data.constBegin(), data.constEnd());

            self->push2MidiOut->sendMessage(&outputMessage);
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
        QByteArray data = QByteArray::fromRawData(reinterpret_cast<const char *>(message->data()),
                                                  static_cast<int>(message->size()));

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
