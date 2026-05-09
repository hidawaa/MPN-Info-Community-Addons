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
    src/homepage_p.h
SOURCES         = \
    src/dialer.cpp \
    src/homepage.cpp \
    src/homepage_t.cpp
DESTDIR         = $$OUT_PWD/../../build/addons
#! [0]

EXAMPLE_FILES = plugin.json

