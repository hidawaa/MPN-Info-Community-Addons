#ifndef SSHEETCELL_H
#define SSHEETCELL_H

#include <QVariant>
#include "ssheetstyle.h"

class QPainter;
class SSheetWorksheet;
class SSheetCell
{
public:
    SSheetCell(SSheetWorksheet *worksheet, quint32 row, quint32 column);

    inline void setStyle(const SSheetStyle &style) { mStyle = style; }
    inline SSheetStyle style() { return mStyle; }

    inline void setRowSpan(int value) { mRowSpan = value; }
    inline int rowSpan() { return mRowSpan; }

    inline void setColumnSpan(int value) { mColumnSpan = value; }
    inline int columnSpan() { return mColumnSpan; }

    inline void setValue(const QVariant &value) { mValue = value; }
    inline QVariant value() { return mValue; }

    inline void hide() { mHide = true; }
    inline void show() { mHide = false; }
    inline bool isHidden() { return mHide; }

    void render(QPainter *painter);

private:
    SSheetWorksheet *mWorksheet;
    quint32 mRow;
    quint32 mColumn;
    int mRowSpan;
    int mColumnSpan;
    SSheetStyle mStyle;
    QVariant mValue;
    bool mHide;

    static int padding;
};

#endif // SSHEETCELL_H
