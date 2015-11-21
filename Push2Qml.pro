TEMPLATE = app

CONFIG += console
QT += qml quick

SOURCES += main.cpp \
    fboquickwindow.cpp \
    fboquickview.cpp \
    pushquickview.cpp \
    pushdisplay.cpp \
    dither.cpp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

HEADERS += \
    fboquickwindow.h \
    fboquickview.h \
    pushquickview.h \
    pushdisplay.h \
    dither.h

win32:LIBS += -L$$PWD -llibusb-1.0
else:LIBS += -lusb-1.0
win32:CONFIG(debug, debug|release): libusb_dll.commands = $(COPY) $$system_path($$PWD/libusb-1.0.dll) $$system_path($$OUT_PWD/debug/libusb-1.0.dll)
win32:CONFIG(release, debug|release): libusb_dll.commands = $(COPY) $$system_path($$PWD/libusb-1.0.dll) $$system_path($$OUT_PWD/release/libusb-1.0.dll)
win32:first.depends = $(first) libusb_dll
win32:export(first.depends)
win32:export(libusb_dll.commands)
win32:QMAKE_EXTRA_TARGETS += first libusb_dll
