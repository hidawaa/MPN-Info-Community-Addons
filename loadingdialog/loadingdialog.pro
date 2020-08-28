#! [0]
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += $$PWD/../../pdk/include
HEADERS         = plugin.h \
    addon.h \
    loadingdialog.h
QT             += sql widgets
SOURCES         = \
    loadingdialog.cpp
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json
