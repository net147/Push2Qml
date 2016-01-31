#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include "pushquickview.h"

#ifdef Q_OS_WIN
#   define PATH_SEPARATOR ";"
#else
#   define PATH_SEPARATOR ":"
#endif

int main(int argc, char *argv[])
{
    {
        QCoreApplication app(argc, argv);

        QDir::setCurrent(app.applicationDirPath());
    }

    qputenv("QML2_IMPORT_PATH", "Push2/qml" PATH_SEPARATOR "Qt/qml");
    qputenv("QT_PLUGIN_PATH", "Qt/plugins");

    QGuiApplication app(argc, argv);
    QCommandLineParser parser;

    parser.setApplicationDescription("Ableton Push 2 QML Runner");
    parser.addHelpOption();

    QCommandLineOption ditherOption("dither",
                                    QCoreApplication::translate("main", "Dither graphics when rendering to BGR565."));

    parser.addOption(ditherOption);
    parser.addPositionalArgument("filename",
                                 QCoreApplication::translate("main", "The file to open (defaults to PushDisplay.qml in"
                                                             " the same directory as this application if it exists"
                                                             " for Ableton Push 1 emulation)."));
    parser.process(app);

    QString filename;

    if (!parser.positionalArguments().empty())
        filename = QDir::fromNativeSeparators(parser.positionalArguments().first());
    else
        filename = app.applicationDirPath() + "/PushDisplay.qml";

    if (!QFileInfo(filename).exists())
        parser.showHelp(0);

    PushQuickView w(QUrl::fromLocalFile(filename));

    w.setDithering(parser.isSet("dither"));

    if (!w.isOpen()) {
        qCritical("Unable to open Push 2 display");
        return 1;
    }

    return app.exec();
}
