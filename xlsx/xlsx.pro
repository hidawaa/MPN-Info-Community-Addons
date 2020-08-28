#! [0]
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += $$PWD/../../pdk/include
HEADERS         = plugin.h \
    xlsxaddon.h
QT             += sql
SOURCES         =
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json

include("$$PWD/ssheet/ssheet.pri")
