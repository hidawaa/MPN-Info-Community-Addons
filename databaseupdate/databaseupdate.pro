#! [0]
TEMPLATE        = lib
CONFIG         += plugin
QT             += sql widgets network
INCLUDEPATH    += $$PWD/../../pdk/include
HEADERS         = plugin.h \
    addons.h \
    databaseupdate.h
SOURCES         =
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json

