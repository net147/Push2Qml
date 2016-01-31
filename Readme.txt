Description:
Run QML on Ableton Push 2 LCD display or emulate Ableton Push 1 using Ableton Push 2.

Pre-built binaries for Windows 64-bit:
https://www.dropbox.com/sh/kqy3pydkpgcjusv/AAD_5q9o58XF5a6WpyGua8Sza

Running Push2Qml with existing Ableton Live 9.5 libraries on Windows 64-bit:
1. Copy Push2Qml.exe executable to C:\ProgramData\Ableton\Live 9 Suite\Program
2. Drag ColorAnimationTest.qml onto Push2Qml.exe. Alternatively, you can use any QML file.

Emulating an Ableton Push 1 using Ableton Push 2:
You need to create 2 MIDI loopback ports with the names "MIDIIN2 (Ableton Push)" and "MIDIOUT2 (Ableton Push)".
This can be done on Windows using loopMIDI.
Run Push2Qml. It will use PushDisplay.qml from the same directory if it exists to emulate the Push 1 display.

Build instructions - Windows 32-bit:
1. Install Microsoft Visual Studio Express 2013 for Windows Desktop with Update 4
2. Install Qt 5.5.0 from http://download.qt.io/archive/qt/5.5/5.5.0/qt-opensource-windows-x86-msvc2013-5.5.0.exe
3. Download libusb-1.0.20.7z from http://libusb.info/
4. Extract include\libusb-1.0\libusb.h, MS32\dll\libusb-1.0.dll and MS32\dll\libusb-1.0.lib into this directory
5. Open Push2Qml.pro using Qt Creator
6. Change the build configuration from Debug to Release
7. Click Build

Build instructions - Windows 64-bit:
1. Install Microsoft Visual Studio Express 2013 for Windows Desktop with Update 4
2. Install Qt 5.5.0 from http://download.qt.io/archive/qt/5.5/5.5.0/qt-opensource-windows-x86-msvc2013_64-5.5.0.exe
3. Download libusb-1.0.20.7z from http://libusb.info/
4. Extract include\libusb-1.0\libusb.h, MS64\dll\libusb-1.0.dll and MS64\dll\libusb-1.0.lib into this directory
5. Open Push2Qml.pro using Qt Creator
6. Change the build configuration from Debug to Release
7. Click Build

Development:
You may want to configure Push2Qml to automatically run a particular QML file when run from Qt Creator.
1. Click Projects on the left sidebar
2. Click the Run tab to access Run Settings
3. Under Run, enter %{sourceDir}/ColorAnimationTest.qml (or any other QML file) in the Arguments text field
4. Clicking the Run icon on the left sidebar will now start Push2Qml with the specified QML file
