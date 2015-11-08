TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
    fboquickwindow.cpp \
    fboquickview.cpp \
    pushquickview.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    fboquickwindow.h \
    fboquickview.h \
    pushquickview.h

LIBS += -L$$PWD -llibusb-1.0
