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

#ifdef Q_OS_WIN
#include <QStandardPaths>
#include <dbghelp.h>
#include <qt_windows.h>
#include <tchar.h>

static TCHAR miniDumpApplicationName[_MAX_PATH];
static TCHAR miniDumpPath[_MAX_PATH];

static LONG WINAPI miniDumpExceptionFilter(PEXCEPTION_POINTERS exceptionPointers)
{
    if (IsDebuggerPresent())
        return EXCEPTION_CONTINUE_SEARCH;

    HMODULE dbgHelp = LoadLibrary(TEXT("dbghelp.dll"));

    if (dbgHelp == NULL)
        return EXCEPTION_EXECUTE_HANDLER;

    HANDLE file = CreateFile(miniDumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return EXCEPTION_EXECUTE_HANDLER;

    typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
    LPMINIDUMPWRITEDUMP miniDumpWriteDump = (LPMINIDUMPWRITEDUMP)GetProcAddress(dbgHelp, "MiniDumpWriteDump");

    if (!miniDumpWriteDump)
        return EXCEPTION_EXECUTE_HANDLER;

    MINIDUMP_EXCEPTION_INFORMATION mei;

    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = exceptionPointers;
    mei.ClientPointers = TRUE;

    BOOL writeDump = miniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, exceptionPointers ? &mei : NULL, NULL, NULL);

    CloseHandle(file);
    FreeLibrary(dbgHelp);

    if (writeDump)
    {
        TCHAR message[_MAX_PATH * 2];
        lstrcpy(message, miniDumpApplicationName);
        lstrcat(message, TEXT(" just crashed. Crash information was saved to the file '"));
        lstrcat(message, miniDumpPath);
        lstrcat(message, TEXT("', please send it to the developers for debugging."));

        MessageBox(NULL, message, miniDumpApplicationName, MB_OK);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
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

    app.setApplicationName("Push2Qml");
#ifdef Q_OS_WIN
    lstrcpy(miniDumpApplicationName, reinterpret_cast<LPCWSTR>(app.applicationName().utf16()));
    QString miniDumpDir;
    miniDumpDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    miniDumpDir += "/Crash Dumps";
    QDir(miniDumpDir).mkpath("...");
    lstrcpy(miniDumpPath, reinterpret_cast<LPCWSTR>(
                QDir::toNativeSeparators(miniDumpDir
                                         + "/" + QFileInfo(qApp->applicationFilePath()).completeBaseName() + ".dmp"
                                         ).utf16()));
    SetUnhandledExceptionFilter(miniDumpExceptionFilter);
#endif

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
