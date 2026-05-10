TEMPLATE = lib
CONFIG += plugin c++17
QT += core gui widgets sql

TARGET = mfwp
DESTDIR = $$OUT_PWD/../../build/addons

INCLUDEPATH += . \
    src \
    ../../pdk/include

HEADERS += \
    mfwpaddon.h \
    plugin.h \
    src/masterfilesearch_p.h \
    src/masterfilesearchpage.h \
    src/kluselectdialog.h

SOURCES += \
    plugin.cpp \
    src/masterfilesearch_p.cpp \
    src/masterfilesearchpage.cpp \
    src/kluselectdialog.cpp

include(QXlsx/QXlsx/QXlsx.pri)

RESOURCES += mfwp.qrc
