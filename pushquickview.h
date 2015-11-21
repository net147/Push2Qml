#ifndef PUSHQUICKVIEW_H
#define PUSHQUICKVIEW_H

#include "fboquickview.h"
#include <QScopedPointer>

class PushQuickViewPrivate;

class PushQuickView : public FboQuickView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PushQuickView)
    Q_PROPERTY(bool dithering READ dithering WRITE setDithering NOTIFY ditheringChanged)

public:
    PushQuickView(const QUrl &url);
    ~PushQuickView();
    bool isOpen() const;
    bool dithering() const;
    void setDithering(bool value);

signals:
    void ditheringChanged();

private:
    const QScopedPointer<PushQuickViewPrivate> d_ptr;
};

#endif // PUSHQUICKVIEW_H
