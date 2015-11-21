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

PushQuickView::PushQuickView(const QUrl &url) :
    d_ptr(new PushQuickViewPrivate(this))
{
    Q_D(PushQuickView);

    if (!isOpen())
        return;

    setGeometry(QRect(QPoint(0, 0), d->display.size()));
    setSource(url);
}

PushQuickView::~PushQuickView()
{
}

bool PushQuickView::isOpen() const
{
    Q_D(const PushQuickView);
    return d->display.isOpen();
}

bool PushQuickView::dithering() const
{
    Q_D(const PushQuickView);
    return d->display.dithering();
}

void PushQuickView::setDithering(bool value)
{
    Q_D(PushQuickView);
    if (d->display.dithering() != value) {
        d->display.setDithering(value);
        emit ditheringChanged();
    }
}

#include "pushquickview.moc"
