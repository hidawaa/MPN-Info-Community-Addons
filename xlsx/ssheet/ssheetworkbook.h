#ifndef SSHEETWORKBOOK_H
#define SSHEETWORKBOOK_H

#include <QFont>
#include <QList>
#include "ssheetstyle.h"

class SSheetWorksheet;
class SSheetWorkbook
{
public:
    explicit SSheetWorkbook();
    ~SSheetWorkbook();

    bool load(const QString &filename);
    bool save(const QString &filename);
    void close();

    QString toHtml();

    SSheetWorksheet *createWorksheet(const QString &name);
    SSheetWorksheet *worksheet(int index);
    SSheetWorksheet *worksheet(const QString &name);
    QStringList worksheetNames();
    int countWorksheet();

private:
    QList<SSheetWorksheet *> mWorksheetList;
};

#endif // SSHEETWORKBOOK_H
