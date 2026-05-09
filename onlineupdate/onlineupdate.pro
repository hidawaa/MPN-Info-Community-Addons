#! [0]
TEMPLATE        = lib
CONFIG         += plugin c++17
QT             += sql widgets network
INCLUDEPATH    += $$PWD/../../pdk/include
HEADERS         = plugin.h
SOURCES         =
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json

