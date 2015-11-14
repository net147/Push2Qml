#include "pushquickview.h"
#include "pushdisplay.h"

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
    PushDisplay display;
    PushQuickView * const q_ptr;
};

PushQuickViewPrivate::PushQuickViewPrivate(PushQuickView *q_ptr) :
    q_ptr(q_ptr)
{
    connect(q_ptr, SIGNAL(sceneRendered()), SLOT(sceneRendered()));
}

PushQuickViewPrivate::~PushQuickViewPrivate()
{
}

void PushQuickViewPrivate::sceneRendered()
{
    Q_Q(PushQuickView);
    display.drawImage(q->grab());
}

void PushQuickViewPrivate::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    sceneRendered();
}

PushQuickView::PushQuickView(const QUrl &url) :
    d_ptr(new PushQuickViewPrivate(this))
{
    Q_D(PushQuickView);
    setGeometry(QRect(QPoint(0, 0), d->display.size()));
    setSource(url);
}

PushQuickView::~PushQuickView()
{
}

#include "pushquickview.moc"
