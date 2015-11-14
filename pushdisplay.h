#ifndef PUSHDISPLAY_H
#define PUSHDISPLAY_H

#include <QObject>
#include <QScopedPointer>

class PushDisplayPrivate;

class PushDisplay : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PushDisplay)

public:
    explicit PushDisplay(QObject *parent = 0);
    ~PushDisplay();
    bool isOpen() const;
    void drawImage(const QImage &image);
    QSize size() const;

private:
    const QScopedPointer<PushDisplayPrivate> d_ptr;
};

#endif // PUSHDISPLAY_H
