QT       -= core gui
QT_PRIVATE += core

TARGET = zlib
TEMPLATE = lib
CONFIG += staticlib c99 warn_off
DESTDIR = $$OUT_PWD/../../build

INCLUDEPATH = $$PWD
SOURCES+= \
    $$PWD/adler32.c \
    $$PWD/compress.c \
    $$PWD/crc32.c \
    $$PWD/deflate.c \
    $$PWD/gzclose.c \
    $$PWD/gzlib.c \
    $$PWD/gzread.c \
    $$PWD/gzwrite.c \
    $$PWD/infback.c \
    $$PWD/inffast.c \
    $$PWD/inflate.c \
    $$PWD/inftrees.c \
    $$PWD/trees.c \
    $$PWD/uncompr.c \
    $$PWD/zutil.c
