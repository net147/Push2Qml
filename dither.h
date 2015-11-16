#ifndef DITHER_H
#define DITHER_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QImage)

QImage ditherToBgr565(const QImage &image);

#endif // DITHER_H

