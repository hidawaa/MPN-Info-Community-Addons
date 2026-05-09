#! [0]
TEMPLATE        = lib
CONFIG         += plugin c++17
QT             += sql widgets network
INCLUDEPATH    += $$PWD/../../pdk/include
HEADERS         = plugin.h \
    addons.h \
    downloader.h
SOURCES         = \
    downloader.cpp
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json

