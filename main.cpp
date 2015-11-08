#include <QGuiApplication>
#include "pushquickview.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    PushQuickView w;

    //QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);

    return app.exec();
}
