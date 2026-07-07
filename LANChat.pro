QT += core gui network widgets

CONFIG += c++17

TEMPLATE = app
TARGET = LANChat

CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/qrc
UI_DIR = build/ui

INCLUDEPATH += include

HEADERS += \
    include/peerinfo.h \
    include/chatmanager.h \
    include/mainwindow.h

SOURCES += \
    src/main.cpp \
    src/chatmanager.cpp \
    src/mainwindow.cpp

FORMS += \
    ui/mainwindow.ui
