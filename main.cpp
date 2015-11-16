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

    parser.setApplicationDescription("Push 2 QML Runner");
    parser.addHelpOption();

    QCommandLineOption ditherOption("dither",
                                    QCoreApplication::translate("main", "Dither graphics when rendering to BGR565."));

    parser.addOption(ditherOption);
    parser.addPositionalArgument("filename", QCoreApplication::translate("main", "The file to open."));
    parser.process(app);

    QString filename;

    if (!parser.positionalArguments().empty())
        filename = QDir::fromNativeSeparators(parser.positionalArguments().first());
    if (!QFileInfo(filename).exists())
        parser.showHelp(0);

    PushQuickView w(QUrl::fromLocalFile(filename));

    w.setDithering(parser.isSet("dither"));
    Q_UNUSED(w);

    return app.exec();
}
