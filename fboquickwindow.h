#ifndef FBOQUICKWINDOW_H
#define FBOQUICKWINDOW_H

#include <QQuickWindow>
#include <QScopedPointer>

class FboQuickWindowPrivate;

class FboQuickWindow : public QQuickWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(FboQuickWindow)

public:
    explicit FboQuickWindow();
    ~FboQuickWindow();

    QImage grab() const;
    void resize(const QSize &newSize);
    void resize(int w, int h);

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void sceneRendered();

private:
    using QQuickWindow::resize;
    FboQuickWindowPrivate *d_ptr;
    Q_DECLARE_PRIVATE(FboQuickWindow)
};

#endif // FBOQUICKWINDOW_H
