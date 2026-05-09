#ifndef SSHEETADDRESS_H
#define SSHEETADDRESS_H

#include <qglobal.h>

class SSheetAddress
{
public:
    SSheetAddress(quint32 r, quint32 c) : row(r), column(c) {}
    SSheetAddress(const SSheetAddress &other) : row(other.row), column(other.column) {}
    SSheetAddress &operator=(const SSheetAddress &other) { row=other.row; column=other.column; return *this; }
    bool operator==(const SSheetAddress &other) { return row==other.row && column==other.column; }

    QString toString();

    static SSheetAddress fromString(const QString &address);

    quint32 row;
    quint32 column;
};

#endif // SSHEETADDRESS_H
