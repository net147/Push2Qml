#include "fboquickwindow.h"
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickRenderControl>
#include <QTimer>

class FboQuickWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FboQuickWindow)
    Q_DISABLE_COPY(FboQuickWindowPrivate)

public:
    FboQuickWindowPrivate(FboQuickWindow *q_ptr);
    ~FboQuickWindowPrivate();

    static FboQuickWindowPrivate *create(FboQuickWindowPrivate *&d_ptr, FboQuickWindow *q_ptr);

public slots:
    void sceneGraphInitialized();
    void sceneGraphInvalidated();
    void renderRequested();
    void sceneChanged();

public:
    void render();

public:
    QOpenGLContext *context;
    QOffscreenSurface *offscreenSurface;
    QQuickRenderControl *renderControl;
    QOpenGLFramebufferObject *fbo;
    bool needsPolishAndSync;
    QTimer updateTimer;

    FboQuickWindow * const q_ptr;
};

FboQuickWindowPrivate::FboQuickWindowPrivate(FboQuickWindow *q_ptr) :
    fbo(0),
    needsPolishAndSync(false),
    q_ptr(q_ptr)
{
    updateTimer.setSingleShot(true);
    updateTimer.setInterval(5);

    connect(&updateTimer, &QTimer::timeout, this, &FboQuickWindowPrivate::render);

    renderControl = new QQuickRenderControl;

    connect(renderControl, &QQuickRenderControl::renderRequested, this, &FboQuickWindowPrivate::renderRequested);
    connect(renderControl, &QQuickRenderControl::sceneChanged, this, &FboQuickWindowPrivate::sceneChanged);

    QSurfaceFormat format;
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);

    context = new QOpenGLContext;
    context->setFormat(format);
    context->create();

    offscreenSurface = new QOffscreenSurface;
    offscreenSurface->setFormat(context->format());
    offscreenSurface->create();
}

FboQuickWindowPrivate::~FboQuickWindowPrivate()
{
    context->makeCurrent(offscreenSurface);
    delete renderControl;
    context->doneCurrent();
    delete offscreenSurface;
    delete context;
}

FboQuickWindowPrivate *FboQuickWindowPrivate::create(FboQuickWindowPrivate *&d_ptr,
                                                     FboQuickWindow *q_ptr)
{
    return (d_ptr = new FboQuickWindowPrivate(q_ptr));
}

void FboQuickWindowPrivate::sceneGraphInitialized()
{
    Q_Q(FboQuickWindow);

    Q_ASSERT(context);
    Q_ASSERT(!fbo);

    fbo = new QOpenGLFramebufferObject(QSize(1, 1).expandedTo(q->size()),
                                       QOpenGLFramebufferObject::CombinedDepthStencil);
    q->setRenderTarget(fbo);
}

void FboQuickWindowPrivate::sceneGraphInvalidated()
{
    delete fbo;
    fbo = 0;
}

void FboQuickWindowPrivate::renderRequested()
{
    if (!updateTimer.isActive())
        updateTimer.start();
}

void FboQuickWindowPrivate::sceneChanged()
{
    needsPolishAndSync = true;

    if (!updateTimer.isActive())
        updateTimer.start();
}

void FboQuickWindowPrivate::render()
{
    Q_Q(FboQuickWindow);

    if (!context->makeCurrent(offscreenSurface))
        return;

    if (needsPolishAndSync) {
        renderControl->polishItems();
        renderControl->sync();
        needsPolishAndSync = false;
    }

    renderControl->render();

    q->resetOpenGLState();
    QOpenGLFramebufferObject::bindDefault();
    context->functions()->glFlush();

    emit q->sceneRendered();
}

FboQuickWindow::FboQuickWindow() :
    QQuickWindow(FboQuickWindowPrivate::create(d_ptr, this)->renderControl)
{
    Q_D(FboQuickWindow);

    connect(this, &QQuickWindow::sceneGraphInitialized, d, &FboQuickWindowPrivate::sceneGraphInitialized);
    connect(this, &QQuickWindow::sceneGraphInvalidated, d, &FboQuickWindowPrivate::sceneGraphInvalidated);
    d->context->makeCurrent(d->offscreenSurface);
    d->renderControl->initialize(d->context);
    d->context->doneCurrent();
}

FboQuickWindow::~FboQuickWindow()
{
    delete d_ptr;
}

QImage FboQuickWindow::grab() const
{
    Q_D(const FboQuickWindow);

    return d->renderControl->grab();
}

void FboQuickWindow::resize(const QSize &newSize)
{
    QResizeEvent event(size(), newSize);

    setGeometry(0, 0, newSize.width(), newSize.height());
    resizeEvent(&event);
}

void FboQuickWindow::resize(int w, int h)
{
    resize(QSize(w, h));
}

void FboQuickWindow::resizeEvent(QResizeEvent *event)
{
    Q_D(FboQuickWindow);

    QQuickWindow::resizeEvent(event);

    Q_ASSERT(d->context);

    if (d->context->makeCurrent(d->offscreenSurface)) {
        d->sceneGraphInvalidated();
        d->sceneGraphInitialized();
        d->context->doneCurrent();
    }
}

#include "fboquickwindow.moc"
