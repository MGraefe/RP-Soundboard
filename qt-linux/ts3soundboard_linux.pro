# Qt project file for linux builds

CONFIG += c++11
linux-g++-32:ARCHID = 32
linux-g++-64:ARCHID = 64
debug:BUILDTYPE = debug
!debug:BUILDTYPE = release

TEMPLATE = lib
TARGET = ts3soundboard_lin$${ARCHID}
DESTDIR = ../bin/$${BUILDTYPE}_lin_$${ARCHID}

QT += core network widgets gui

DEFINES += LINUX QT_DLL QT_WIDGETS_LIB
debug:DEFINES += _DEBUG

INCLUDEPATH += ../include
    
LIBS += -lavcodec \
    -lavformat \
    -lavutil \
    -lswresample

include(./ts3soundboard.pri)
