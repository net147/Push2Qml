#ifndef FBOQUICKVIEW_H
#define FBOQUICKVIEW_H

#include "fboquickwindow.h"
#include <QList>
#include <QScopedPointer>
#include <QQmlComponent>
#include <QQmlError>

QT_FORWARD_DECLARE_CLASS(QQmlContext)

class FboQuickViewPrivate;

class FboQuickView : public FboQuickWindow
{
    Q_OBJECT

public:
    FboQuickView();
    ~FboQuickView();

    QQmlContext *rootContext() const;
    QList<QQmlError> errors() const;

    enum Status { Null, Ready, Loading, Error };
    Status status() const;

public slots:
    void setSource(const QUrl &url);

signals:
    void statusChanged(Status status);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    const QScopedPointer<FboQuickViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FboQuickView)
};

#endif // FBOQUICKVIEW_H
