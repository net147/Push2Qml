TEMPLATE = app

CONFIG += console
QT += qml quick

SOURCES += main.cpp \
    fboquickwindow.cpp \
    fboquickview.cpp \
    pushquickview.cpp \
    pushdisplay.cpp \
    dither.cpp \
    rtmidi.cpp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

HEADERS += \
    fboquickwindow.h \
    fboquickview.h \
    pushquickview.h \
    pushdisplay.h \
    dither.h \
    rtmidi.h

mac {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
} else:unix {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound -lpthread
} else:win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
}
win32:LIBS += -L$$PWD -llibusb-1.0 -luser32
else:LIBS += -lusb-1.0

win32 {
    CONFIG(debug, debug|release): libusb_dll.commands = $(COPY) $$system_path($$PWD/libusb-1.0.dll) $$system_path($$OUT_PWD/debug/libusb-1.0.dll)
    CONFIG(release, debug|release): libusb_dll.commands = $(COPY) $$system_path($$PWD/libusb-1.0.dll) $$system_path($$OUT_PWD/release/libusb-1.0.dll)
    first.depends = $(first) libusb_dll
    export(first.depends)
    export(libusb_dll.commands)
    QMAKE_EXTRA_TARGETS += first libusb_dll
}

msvc {
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CFLAGS_RELEASE += /Od /Zi
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += /Od /Zi
    QMAKE_LFLAGS_RELEASE += /map /debug
}
