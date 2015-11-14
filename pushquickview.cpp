#include "pushquickview.h"
#include "libusb.h"
#include <QDebug>
#include <algorithm>

class PushQuickViewPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(PushQuickView)

public:
    PushQuickViewPrivate(PushQuickView *q_ptr);
    ~PushQuickViewPrivate();

public slots:
    void sceneRendered();

protected:
    void timerEvent(QTimerEvent *event);

public:
    libusb_context *context;
    libusb_device_handle *device;
    PushQuickView * const q_ptr;
};

PushQuickViewPrivate::PushQuickViewPrivate(PushQuickView *q_ptr) :
    context(0),
    device(0),
    q_ptr(q_ptr)
{
    connect(q_ptr, SIGNAL(sceneRendered()), SLOT(sceneRendered()), Qt::DirectConnection);
    libusb_init(&context);
    device = libusb_open_device_with_vid_pid(context, 0x2982, 0x1967);

    if (device)
        libusb_claim_interface(device, 0);

    // Push display will automatically clear to black if another frame has not been received within 2 seconds
    startTimer(1000);
}

PushQuickViewPrivate::~PushQuickViewPrivate()
{
    if (device) {
        libusb_release_interface(device, 0);
        libusb_close(device);
    }

    if (context)
        libusb_exit(context);
}

void PushQuickViewPrivate::sceneRendered()
{
    QImage image = q_ptr->grab().convertToFormat(QImage::Format_RGB16);
    uchar *bits = image.bits();
    quint16 *pixels = reinterpret_cast<quint16 *>(bits);

    for (int i = 0; i < image.byteCount() / 2; ++i) {
        // Swap RGB565 to BGR565
        pixels[i] = ((pixels[i] & 0x1F) << 11) | (pixels[i] & 0x07E0) | ((pixels[i] & 0xF800) >> 11);
    }

    unsigned char header[] = { 0xEF, 0xCD, 0xAB, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int transferred = 0;

    // Transfer header which indicates start of image transfer
    libusb_bulk_transfer(device, 0x01 | LIBUSB_ENDPOINT_OUT, header, sizeof(header), &transferred, 3000);

    // Transfer 20 blocks of 1024 x 8 pixels which make up a 1024 x 160 pixel screen image
    for (int i = 0; i < 20; ++i) {
        unsigned char *data = const_cast<unsigned char *>(bits);

        libusb_bulk_transfer(device, 0x01 | LIBUSB_ENDPOINT_OUT, data + 16384 * i, 16384, &transferred, 3000);
    }
}

void PushQuickViewPrivate::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    sceneRendered();
}

PushQuickView::PushQuickView(const QUrl &url) :
    d_ptr(new PushQuickViewPrivate(this))
{
    setGeometry(0, 0, 1024, 160);
    setSource(url);
}

PushQuickView::~PushQuickView()
{
}

#include "pushquickview.moc"
