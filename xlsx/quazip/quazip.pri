DEFINES += NOMINMAX QUAZIP_BUILD QUAZIP_STATIC ZLIB_INTERNAL
INCLUDEPATH += $$PWD
CONFIG += warn_off

msvc {
    INCLUDEPATH += $$PWD/../zlib
    SOURCES += \
        $$PWD/../zlib/adler32.c \
        $$PWD/../zlib/compress.c \
        $$PWD/../zlib/crc32.c \
        $$PWD/../zlib/deflate.c \
        $$PWD/../zlib/gzclose.c \
        $$PWD/../zlib/gzlib.c \
        $$PWD/../zlib/gzread.c \
        $$PWD/../zlib/gzwrite.c \
        $$PWD/../zlib/infback.c \
        $$PWD/../zlib/inffast.c \
        $$PWD/../zlib/inflate.c \
        $$PWD/../zlib/inftrees.c \
        $$PWD/../zlib/trees.c \
        $$PWD/../zlib/uncompr.c \
        $$PWD/../zlib/zutil.c

    # Suppress LNK4217/LNK4286: zlib symbols defined locally but referenced
    # as imports — harmless when zlib source is compiled directly into the plugin.
    QMAKE_LFLAGS += /ignore:4217 /ignore:4286
}
else {
    LIBS += -lz
}

HEADERS += \
        $$PWD/crypt.h \
        $$PWD/ioapi.h \
        $$PWD/JlCompress.h \
        $$PWD/quaadler32.h \
        $$PWD/quachecksum32.h \
        $$PWD/quacrc32.h \
        $$PWD/quagzipfile.h \
        $$PWD/quaziodevice.h \
        $$PWD/quazipdir.h \
        $$PWD/quazipfile.h \
        $$PWD/quazipfileinfo.h \
        $$PWD/quazip_global.h \
        $$PWD/quazip.h \
        $$PWD/quazipnewinfo.h \
        $$PWD/unzip.h \
        $$PWD/zip.h

SOURCES += $$PWD/qioapi.cpp \
           $$PWD/JlCompress.cpp \
           $$PWD/quaadler32.cpp \
           $$PWD/quacrc32.cpp \
           $$PWD/quagzipfile.cpp \
           $$PWD/quaziodevice.cpp \
           $$PWD/quazip.cpp \
           $$PWD/quazipdir.cpp \
           $$PWD/quazipfile.cpp \
           $$PWD/quazipfileinfo.cpp \
           $$PWD/quazipnewinfo.cpp \
           $$PWD/unzip.c \
           $$PWD/zip.c
