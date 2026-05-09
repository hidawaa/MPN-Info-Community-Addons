#include "ssheetcell.h"

#include <QFontMetrics>
#include <QPainter>
#include <QDebug>

#include "ssheetworkbook.h"
#include "ssheetworksheet.h"

int SSheetCell::padding(0);

SSheetCell::SSheetCell(SSheetWorksheet *worksheet, quint32 row, quint32 column) :
    mWorksheet(worksheet),
    mRow(row),
    mColumn(column),
    mRowSpan(1),
    mColumnSpan(1),
    mHide(false) {}

void SSheetCell::render(QPainter *painter)
{
    if (!painter)
        return;

    int height = 0;
    for (quint32 row=mRow; row<mRow+quint32(mRowSpan); row++)
        height += mWorksheet->rowHeight(row);

    int width = 0;
    for (quint32 column=mColumn; column<mColumn+quint32(mColumnSpan); column++)
        width += mWorksheet->columnWidth(column);

    for (int borderPosition=1; borderPosition<=4; borderPosition++) {
        SSheetBorder::Type borderType(SSheetBorder::None);
        QPointF startPoint, endPoint;

        if (borderPosition == SSheetBorder::Top) {
            borderType = mStyle.border.top;
            startPoint = QPointF(0, 0);
            endPoint = QPointF(width, 0);
        }
        else if (borderPosition == SSheetBorder::Bottom) {
            borderType = mStyle.border.bottom;
            startPoint = QPointF(0, height);
            endPoint = QPointF(width, height);
        }
        else if (borderPosition == SSheetBorder::Left) {
            startPoint = QPointF(0, 0);
            borderType = mStyle.border.left;
            endPoint = QPointF(0, height);
        }
        else if (borderPosition == SSheetBorder::Right) {
            borderType = mStyle.border.right;
            startPoint = QPointF(width, 0);
            endPoint = QPointF(width, height);
        }

        QPen pen;
        if (borderType == SSheetBorder::Thin) {
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(1);
        }
        else if (borderType == SSheetBorder::Medium) {
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(2);
        }
        else if (borderType == SSheetBorder::Thick) {
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(3);
        }
        else if (borderType == SSheetBorder::Dot) {
            pen.setStyle(Qt::DotLine);
            pen.setWidth(1);
        }
        else if (borderType == SSheetBorder::Dash) {
            pen.setStyle(Qt::DashLine);
            pen.setWidth(1);
        }
        else if (borderType == SSheetBorder::DashDot) {
            pen.setStyle(Qt::DashDotLine);
            pen.setWidth(1);
        }
        else if (borderType == SSheetBorder::DashDotDot) {
            pen.setStyle(Qt::DashDotDotLine);
            pen.setWidth(1);
        }

        if (borderType == SSheetBorder::None)
            continue;

        painter->save();
        painter->setPen(pen);
        painter->drawLine(startPoint, endPoint);
        painter->restore();
    }

    height -= (2 + padding) * 2;
    width -= (2 + padding) * 2;

    painter->save();
    QString text = mValue.toString();
    painter->setFont(mStyle.font);

    int flag = mStyle.horizontalAlignment | mStyle.verticalAlignment;
    if (mStyle.wrapText)
        flag |= Qt::TextWordWrap;

    QFontMetrics metrics = painter->fontMetrics();
    QRect boundRect = metrics.boundingRect(0, 0, width, height, flag, text);
    quint32 overlapColumn(mColumn);
    while (!mStyle.wrapText && width < boundRect.width()) {
        if (!mWorksheet->cell(mRow, ++overlapColumn)->value().toString().isEmpty())
            break;

        width += mWorksheet->columnWidth(overlapColumn);
    }

    painter->translate(2 + padding, 2 + padding);
    painter->drawText(0, 0, width, height + 2, flag, text);
    painter->restore();
}
