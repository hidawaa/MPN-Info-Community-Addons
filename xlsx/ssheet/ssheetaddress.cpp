#include "ssheetaddress.h"

#include <QString>

quint32 _letterToInt(const QString &letter)
{
    quint32 number(0);

    for (int i=0; i < letter.length(); i++)
        number = number * 26 + (letter.at(i).toLatin1() - ('A' - 1));

    return number - 1;
}

QString _intToLetter(quint32 _number)
{
    quint32 number = _number + 1;
    QString result;

    while (number-- > 0) {
        result.prepend((QChar)('A' + char(number % 26)));
        number /= 26;
    }

    return result;
}

QString SSheetAddress::toString()
{
    return QString("%1%2").arg(_intToLetter(column)).arg(row + 1);
}

SSheetAddress SSheetAddress::fromString(const QString &address)
{
    QString row, column;
    for (int i=0; i < address.length(); i++) {
        QChar r = address.at(i);
        if (r.isLetter())
            column.append(r);
        else
            row.append(r);
    }

    return SSheetAddress(row.toInt() - 1, _letterToInt(column));
}
