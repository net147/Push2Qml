#include "pushdisplay.h"
#include "dither.h"
#include <QImage>
#include <QPainter>

#if defined(Q_CC_MSVC)
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_MSVC(4200)
#   include "libusb.h"
    QT_WARNING_POP
#endif

class PushDisplayPrivate : public QObject
{
    Q_OBJECT

public:
    PushDisplayPrivate();
    ~PushDisplayPrivate();

    void drawImage(const QImage &image);
    void drawNativeImage(const QImage &image);
    QSize paddedSize() const;
    QSize size() const;

protected:
    void timerEvent(QTimerEvent *event);

public:
    libusb_context *context;
    libusb_device_handle *device;
    QImage currentImage;
    bool dithering;
};

PushDisplayPrivate::PushDisplayPrivate() :
    dithering(false)
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
    QImage source = image;

    if (source.isNull()) {
        source = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::black);
    }

    if (dithering) {
        // Scale and dither to BGR565
        drawNativeImage(ditherToBgr565(source.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    } else{
        // Scale and convert to BGR565
        drawNativeImage(source.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                        .convertToFormat(QImage::Format_RGB16).rgbSwapped());
    }
}

void PushDisplayPrivate::drawNativeImage(const QImage &image)
{
    if (!device)
        return;

    Q_ASSERT(image.size() == size());
    Q_ASSERT(image.format() == QImage::Format_RGB16);

    // Each horizontal line is 960 pixels but we need to pad it to 1024 pixels
    QImage paddedImage(paddedSize(), image.format());
    QPainter painter;

    paddedImage.fill(Qt::black);
    painter.begin(&paddedImage);
    painter.drawImage(QPoint(0, 0), image);
    painter.end();

    Q_ASSERT(paddedImage.byteCount() == 20 * 16384);

    unsigned char header[] = { 0xEF, 0xCD, 0xAB, 0x89, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int transferred = 0;

    // Transfer header which indicates start of image transfer
    libusb_bulk_transfer(device, 0x01 | LIBUSB_ENDPOINT_OUT, header, sizeof(header), &transferred, 3000);

    // Transfer image
    libusb_bulk_transfer(device, 0x01 | LIBUSB_ENDPOINT_OUT, const_cast<uchar *>(paddedImage.bits()),
                         20 * 16384, &transferred, 3000);
}

QSize PushDisplayPrivate::paddedSize() const
{
    return QSize(1024, 160);
}

QSize PushDisplayPrivate::size() const
{
    return QSize(960, 160);
}

void PushDisplayPrivate::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    drawImage(currentImage);
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

bool PushDisplay::dithering() const
{
    Q_D(const PushDisplay);
    return d->dithering;
}

void PushDisplay::setDithering(bool value)
{
    Q_D(PushDisplay);
    if (d->dithering != value) {
        d->dithering = value;
        d->drawImage(d->currentImage);
        emit ditheringChanged();
    }
}

void PushDisplay::drawImage(const QImage &image)
{
    Q_D(PushDisplay);
    d->currentImage = image;
    d->drawImage(d->currentImage);
}

QSize PushDisplay::size() const
{
    Q_D(const PushDisplay);
    return d->size();
}

#include "pushdisplay.moc"
