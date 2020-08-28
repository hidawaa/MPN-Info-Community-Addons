#ifndef SSHEETPRINTER_H
#define SSHEETPRINTER_H

#include <QPrinter>

class SSheetWorksheet;
class SSheetPrinter
{
public:
    SSheetPrinter(SSheetWorksheet *worksheet);

    void setPaperSize(QPrinter::PaperSize size);
    void setMargin(int value);
    void setMargin(int top, int bottom, int left, int right);
    void saveAsPdf(const QString &filename);
    void print();

private:
    void render(QPrinter *printer);

private:
    SSheetWorksheet *mWorksheet;
    QPrinter mPrinter;
    int mPaperSize;
    int mMarginTop;
    int mMarginBottom;
    int mMarginLeft;
    int mMarginRight;
};

#endif // SSHEETPRINTER_H
