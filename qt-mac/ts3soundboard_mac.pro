# Qt project file for linux builds

QT_CONFIG -= no-pkg-config

CONFIG += c++11 plugin link_pkgconfig
linux-g++-32:ARCHID = 32
linux-g++-64:ARCHID = 64
CONFIG(debug, release|debug):BUILDTYPE = "debug"
CONFIG(release, release|debug):BUILDTYPE = "release"
TEMPLATE = lib
TARGET = rp_soundboard
DESTDIR = ../bin/$${BUILDTYPE}_lin

QT += core network widgets gui

DEFINES += LINUX QT_DLL QT_WIDGETS_LIB
CONFIG(debug, release|debug):DEFINES += _DEBUG
CONFIG(release, release|debug):DEFINES += NDEBUG

INCLUDEPATH += ../include

LIBS += "-L/Applications/TeamSpeak 3 Client.app/Contents/SharedSupport/update.app/Contents/Frameworks/"

PKGCONFIG += libavcodec libavutil libavformat libswresample

QMAKE_POST_LINK += "cp $${DESTDIR}/lib$${TARGET}.dylib ~/Library/Application\ Support/TeamSpeak\ 3/plugins"


include(../qt-linux/ts3soundboard.pri)
