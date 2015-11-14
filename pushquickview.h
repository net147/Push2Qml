#ifndef PUSHQUICKVIEW_H
#define PUSHQUICKVIEW_H

#include "fboquickview.h"
#include <QScopedPointer>

class PushQuickViewPrivate;

class PushQuickView : public FboQuickView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PushQuickView)

public:
    PushQuickView(const QUrl &url);
    ~PushQuickView();

private:
    const QScopedPointer<PushQuickViewPrivate> d_ptr;
};

#endif // PUSHQUICKVIEW_H
