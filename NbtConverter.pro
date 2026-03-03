QT       += core gui widgets

TARGET = NbtConverter
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h nbt_handler.h \
    nbt_handler.h
FORMS   += mainwindow.ui

RESOURCES += \
    resources.qrc
RC_ICONS = icons/NbtConverter.ico


