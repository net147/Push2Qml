#include "fboquickview.h"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>

class FboQuickViewPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FboQuickView)

public:
    FboQuickViewPrivate(FboQuickView *q_ptr);
    ~FboQuickViewPrivate();

    void setSource(const QUrl &source);

public slots:
    void componentStatusChanged(QQmlComponent::Status status);

public:
    QQmlEngine qmlEngine;
    QQmlComponent *qmlComponent;
    QQuickItem *rootItem;

    FboQuickView *q_ptr;
};

FboQuickViewPrivate::FboQuickViewPrivate(FboQuickView *q_ptr) :
    qmlComponent(0),
    rootItem(0),
    q_ptr(q_ptr)
{
}

FboQuickViewPrivate::~FboQuickViewPrivate()
{
}

void FboQuickViewPrivate::setSource(const QUrl &source)
{
    if (rootItem) {
        delete rootItem;
        rootItem = 0;
    }

    if (qmlComponent) {
        delete qmlComponent;
        qmlComponent = 0;
    }

    if (!source.isEmpty()) {
        qmlComponent = new QQmlComponent(&qmlEngine, &qmlEngine);
        connect(qmlComponent, &QQmlComponent::statusChanged,
                this, &FboQuickViewPrivate::componentStatusChanged);
        qmlComponent->loadUrl(source);
    }
}

void FboQuickViewPrivate::componentStatusChanged(QQmlComponent::Status status)
{
    Q_Q(FboQuickView);

    Q_ASSERT(!rootItem);

    if (status != QQmlComponent::Ready) {
        emit q->statusChanged(FboQuickView::Status(status));
        return;
    }

    QScopedPointer<QObject> rootObject(qmlComponent->create());

    rootItem = qobject_cast<QQuickItem *>(rootObject.data());

    if (!rootItem) {
        emit q->statusChanged(FboQuickView::Error);
        return;
    }

    q->contentItem()->setFocus(true);
    rootObject.take()->setParent(q->contentItem());
    rootItem->setParentItem(q->contentItem());

    QResizeEvent event(q->size(), q->size());

    q->resizeEvent(&event);

    emit q->statusChanged(FboQuickView::Ready);
}

FboQuickView::FboQuickView() :
    d_ptr(new FboQuickViewPrivate(this))
{
}

FboQuickView::~FboQuickView()
{
}

QQmlContext *FboQuickView::rootContext() const
{
    Q_D(const FboQuickView);
    return d->qmlEngine.rootContext();
}

QList<QQmlError> FboQuickView::errors() const
{
    Q_D(const FboQuickView);
    return d->qmlComponent->errors();
}

FboQuickView::Status FboQuickView::status() const
{
    Q_D(const FboQuickView);
    return FboQuickView::Status(d->qmlComponent->status());
}

void FboQuickView::setSource(const QUrl &url)
{
    Q_D(FboQuickView);
    d->setSource(url);
}

void FboQuickView::resizeEvent(QResizeEvent *event)
{
    Q_D(FboQuickView);

    FboQuickWindow::resizeEvent(event);

    if (d->rootItem)
        d->rootItem->setSize(QSizeF(width(), height()));
}

#include "fboquickview.moc"
