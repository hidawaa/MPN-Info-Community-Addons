QT += printsupport
INCLUDEPATH += $$PWD

SOURCES += \
        $$PWD/ssheetworkbook.cpp \
        $$PWD/ssheetworksheet.cpp \
        $$PWD/ssheetcell.cpp \
        $$PWD/ssheetaddress.cpp \
        $$PWD/ssheetstyle.cpp \
        $$PWD/ssheetborder.cpp \
        $$PWD/ssheetdrawing.cpp \
        $$PWD/ssheetprinter.cpp

HEADERS += \
        $$PWD/ssheetworkbook.h \
        $$PWD/ssheetworksheet.h \
        $$PWD/ssheetcell.h \
        $$PWD/ssheetaddress.h \
        $$PWD/ssheetstyle.h \
        $$PWD/ssheetborder.h \
        $$PWD/ssheetdrawing.h \
        $$PWD/ssheetprinter.h

include("../quazip/quazip.pri")
