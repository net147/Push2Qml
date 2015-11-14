TEMPLATE = app

CONFIG += console
QT += qml quick

SOURCES += main.cpp \
    fboquickwindow.cpp \
    fboquickview.cpp \
    pushquickview.cpp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

HEADERS += \
    fboquickwindow.h \
    fboquickview.h \
    pushquickview.h

LIBS += -L$$PWD -llibusb-1.0
CONFIG(debug, debug|release): libusb_dll.commands = $(COPY) $$system_path($$PWD/libusb-1.0.dll) $$system_path($$OUT_PWD/debug/libusb-1.0.dll)
CONFIG(release, debug|release): libusb_dll.commands = $(COPY) $$system_path($$PWD/libusb-1.0.dll) $$system_path($$OUT_PWD/release/libusb-1.0.dll)
first.depends = $(first) libusb_dll
export(first.depends)
export(libusb_dll.commands)
QMAKE_EXTRA_TARGETS += first libusb_dll
