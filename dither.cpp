#include "dither.h"
#include <QImage>

// Source: http://stackoverflow.com/a/17438757

static const uchar ditherThresholdR[64] = {
    1, 7, 3, 5, 0, 8, 2, 6,
    7, 1, 5, 3, 8, 0, 6, 2,
    3, 5, 0, 8, 2, 6, 1, 7,
    5, 3, 8, 0, 6, 2, 7, 1,

    0, 8, 2, 6, 1, 7, 3, 5,
    8, 0, 6, 2, 7, 1, 5, 3,
    2, 6, 1, 7, 3, 5, 0, 8,
    6, 2, 7, 1, 5, 3, 8, 0
};

static const uchar ditherThresholdG[64] = {
    1, 3, 2, 2, 3, 1, 2, 2,
    2, 2, 0, 4, 2, 2, 4, 0,
    3, 1, 2, 2, 1, 3, 2, 2,
    2, 2, 4, 0, 2, 2, 0, 4,

    1, 3, 2, 2, 3, 1, 2, 2,
    2, 2, 0, 4, 2, 2, 4, 0,
    3, 1, 2, 2, 1, 3, 2, 2,
    2, 2, 4, 0, 2, 2, 0, 4
};

static const uchar ditherThresholdB[64] = {
    5, 3, 8, 0, 6, 2, 7, 1,
    3, 5, 0, 8, 2, 6, 1, 7,
    8, 0, 6, 2, 7, 1, 5, 3,
    0, 8, 2, 6, 1, 7, 3, 5,

    6, 2, 7, 1, 5, 3, 8, 0,
    2, 6, 1, 7, 3, 5, 0, 8,
    7, 1, 5, 3, 8, 0, 6, 2,
    1, 7, 3, 5, 0, 8, 2, 6
};

QImage ditherToBgr565(const QImage &image)
{
    QImage source = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage dithered(source.size(), QImage::Format_RGB16);
    int width = source.width();
    int height = source.height();

    for (int y = 0; y < height; ++y) {
        const QRgb *sourceLine = reinterpret_cast<const QRgb *>(source.constScanLine(y));
        quint16 *destinationLine = reinterpret_cast<quint16 *>(dithered.scanLine(y));

        for (int x = 0; x < width; ++x) {
            quint8 thresholdIndex = ((y & 7) << 3) + (x & 7);
            int r = qRed(sourceLine[x]);
            int g = qGreen(sourceLine[x]);
            int b = qBlue(sourceLine[x]);

            r = (qMin(r + ditherThresholdR[thresholdIndex], 255) >> 3 << 3);
            g = (qMin(g + ditherThresholdG[thresholdIndex], 255) >> 2 << 2);
            b = (qMin(b + ditherThresholdB[thresholdIndex], 255) >> 3 << 3);
            destinationLine[x] = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
        }
    }

    return dithered;
}
