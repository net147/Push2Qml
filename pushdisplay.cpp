#include "pushdisplay.h"
#include "libusb.h"
#include <QImage>

class PushDisplayPrivate : public QObject
{
    Q_OBJECT

public:
    PushDisplayPrivate();
    ~PushDisplayPrivate();

    void drawImage(const QImage &image);
    void drawNativeImage(const QImage &image);
    QSize size() const;

protected:
    void timerEvent(QTimerEvent *event);

public:
    libusb_context *context;
    libusb_device_handle *device;
    QImage currentImage;
};

PushDisplayPrivate::PushDisplayPrivate()
{
    libusb_init(&context);
    device = libusb_open_device_with_vid_pid(context, 0x2982, 0x1967);

    if (device)
        libusb_claim_interface(device, 0);
    else
        qFatal("Unable to open Push 2 display");

    // Push display will automatically clear to black if another frame has not been received within 2 seconds
    startTimer(1000);
}

PushDisplayPrivate::~PushDisplayPrivate()
{
    if (device) {
        libusb_release_interface(device, 0);
        libusb_close(device);
    }

    if (context)
        libusb_exit(context);
}

void PushDisplayPrivate::drawImage(const QImage &image)
{
    // Scale and convert to BGR565
    drawNativeImage(image.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                    .convertToFormat(QImage::Format_RGB16).rgbSwapped());
}

void PushDisplayPrivate::drawNativeImage(const QImage &image)
{
    if (!device)
        return;

    Q_ASSERT(image.size() == size());
    Q_ASSERT(image.format() == QImage::Format_RGB16);

    const uchar *bits = image.bits();
    unsigned char header[] = { 0xEF, 0xCD, 0xAB, 0x89, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int transferred = 0;

    // Transfer header which indicates start of image transfer
    libusb_bulk_transfer(device, 0x01 | LIBUSB_ENDPOINT_OUT, header, sizeof(header), &transferred, 3000);

    // Transfer 20 blocks of 1024 x 8 pixels which make up a 1024 x 160 pixel screen image
    for (int i = 0; i < 20; ++i) {
        unsigned char *data = const_cast<unsigned char *>(bits);

        libusb_bulk_transfer(device, 0x01 | LIBUSB_ENDPOINT_OUT, data + 16384 * i, 16384, &transferred, 3000);
    }
}

QSize PushDisplayPrivate::size() const
{
    return QSize(1024, 160);
}

void PushDisplayPrivate::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    drawNativeImage(currentImage);
}

PushDisplay::PushDisplay(QObject *parent) :
    QObject(parent),
    d_ptr(new PushDisplayPrivate)
{
}

PushDisplay::~PushDisplay()
{
}

bool PushDisplay::isOpen() const
{
    Q_D(const PushDisplay);
    return d->device != 0;
}

void PushDisplay::drawImage(const QImage &image)
{
    Q_D(PushDisplay);
    d->drawImage(image);
}

QSize PushDisplay::size() const
{
    Q_D(const PushDisplay);
    return d->size();
}

#include "pushdisplay.moc"
