#ifndef SSHEETWORKSHEET_H
#define SSHEETWORKSHEET_H

#include <QHash>
#include "ssheetaddress.h"
#include "ssheetcell.h"
#include "ssheetdrawing.h"

class SSheetWorkbook;
class SSheetWorksheet
{
public:
    SSheetWorksheet(SSheetWorkbook *workbook, const QString &name);
    ~SSheetWorksheet();

    inline const QString &name() { return mName; }

    inline SSheetWorkbook *workbook() { return mWorkbook; }
    SSheetCell *cell(const QString &address);
    SSheetCell *cell(quint32 row, quint32 column);

    void addDrawing(const QList<SSheetDrawing> &drawingList);
    void addDrawing(const SSheetDrawing &drawing);

    inline quint32 lastRow() { return mLastRow; }
    inline quint32 lastColumn() { return mLastColumn; }

    inline void setColumnWidth(quint32 column, int width) { mColumnWidthMap[column] = width; }
    inline void setRowHeight(quint32 row, int height) { mRowHeightMap[row] = height; }
    int columnWidth(quint32 column);
    int rowHeight(quint32 row);

    inline int defaultColummnWidth() { return mDefaultColumnWidth; }
    inline int defaultRowHeight() { return mDefaultRowHeight; }

    inline void setDefaultColumnWidth(int value) { mDefaultColumnWidth = value; }
    inline void setDefaultRowHeight(int value) { mDefaultRowHeight = value; }

    int width(quint32 fromColumn, quint32 toColumn);
    int width();
    int height(quint32 fromRow, quint32 toRow);
    int height();

    QString toHtml();

    void render(QPainter *painter);
    void render(QPainter *painter, const QString &from, const QString &to);
    void render(QPainter *painter, quint32 fromRow, quint32 fromColumn, quint32 toRow, quint32 toColumn);

private:
    SSheetWorkbook *mWorkbook;
    QString mName;
    QHash<QString, SSheetCell *> mCellHash;
    quint32 mLastRow;
    quint32 mLastColumn;
    int mDefaultColumnWidth;
    int mDefaultRowHeight;
    QMap<quint32, int> mColumnWidthMap;
    QMap<quint32, int> mRowHeightMap;
    QList<SSheetDrawing> mDrawingList;
};

#endif // SSHEETWORKSHEET_H
