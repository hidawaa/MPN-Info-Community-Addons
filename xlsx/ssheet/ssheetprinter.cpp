#include "ssheetprinter.h"

#include <QPainter>

#include "ssheetworksheet.h"

SSheetPrinter::SSheetPrinter(SSheetWorksheet *worksheet) :
    mWorksheet(worksheet),
    mPaperSize(QPageSize::A4),
    mMarginTop(20),
    mMarginBottom(20),
    mMarginLeft(20),
    mMarginRight(20)
{
}

void SSheetPrinter::setPaperSize(QPageSize::PageSizeId size)
{
    mPaperSize = size;
}

void SSheetPrinter::setMargin(int value)
{
    mMarginTop = value;
    mMarginBottom = value;
    mMarginLeft = value;
    mMarginRight = value;
}

void SSheetPrinter::setMargin(int top, int bottom, int left, int right)
{
    mMarginTop = top;
    mMarginBottom = bottom;
    mMarginLeft = left;
    mMarginRight = right;
}

void SSheetPrinter::render(QPrinter *printer)
{
    QPainter p(printer);
    int dWidth = p.device()->width() - mMarginLeft - mMarginRight;
    int dHeight = p.device()->height() - mMarginTop - mMarginBottom;

    int width = mWorksheet->width();
    qreal scale = dWidth * 1. / width;

    quint32 fromRow(0), toRow(0);
    int dsHeight = qRound(dHeight / scale);
    while (toRow < mWorksheet->lastRow()) {
        if (fromRow != 0)
            printer->newPage();

        int tempHeight(0);
        quint32 tempRow(toRow);
        do {
            tempHeight += mWorksheet->rowHeight(toRow);
            if (tempHeight > dsHeight)
                break;

            toRow = tempRow;
            if (toRow == mWorksheet->lastRow())
                break;

            tempRow++;
        } while (true);

        p.save();
        p.translate(mMarginLeft, mMarginTop);
        p.scale(scale, scale);
        mWorksheet->render(&p, fromRow, 0, toRow, mWorksheet->lastColumn());
        p.restore();

        fromRow = toRow + 1;
        toRow = fromRow;
    }

    p.end();
}

void SSheetPrinter::print()
{
    QPrinter printer;
    render(&printer);
}

void SSheetPrinter::saveAsPdf(const QString &filename)
{
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    render(&printer);
}
