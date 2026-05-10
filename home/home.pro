#! [0]
TEMPLATE        = lib
CONFIG         += plugin c++17
QT             += widgets sql charts
INCLUDEPATH    += $$PWD/../../pdk/include
HEADERS         = \
    plugin.h \
    addons.h \
    src/dialer.h \
    src/homepage.h \
    src/homepage_p.h \
    src/customchartview.h \
    src/chartcalloutitem.h \
    src/penerimaandialer.h \
    src/standarditemdelegate.h
SOURCES         = \
    src/dialer.cpp \
    src/homepage.cpp \
    src/customchartview.cpp \
    src/chartcalloutitem.cpp \
    src/penerimaandialer.cpp \
    src/standarditemdelegate.cpp
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json

