#include "ssheetworksheet.h"

#include <QDate>
#include <QPainter>
#include <QXmlStreamWriter>
#include <QDebug>

#include "ssheetcell.h"

SSheetWorksheet::SSheetWorksheet(SSheetWorkbook *workbook, const QString &name) :
    mWorkbook(workbook),
    mName(name),
    mLastRow(0),
    mLastColumn(0),
    mDefaultColumnWidth(64),
    mDefaultRowHeight(20) {}

SSheetWorksheet::~SSheetWorksheet()
{
    QMutableHashIterator<QString, SSheetCell *> iterator(mCellHash);
    while (iterator.hasNext()) {
        iterator.next();
        delete iterator.value();
    }
    mCellHash.clear();
}

int SSheetWorksheet::columnWidth(quint32 column)
{
    int width = mColumnWidthMap[column];
    if (width == 0)
        width = mDefaultColumnWidth;

    if (width < 3)
        width = 3;

    return width;
}

int SSheetWorksheet::rowHeight(quint32 row)
{
    int height = mRowHeightMap[row];
    if (height == 0)
        height = mDefaultRowHeight;

    if (height < 3)
        height = 3;

    return height;
}

SSheetCell *SSheetWorksheet::cell(const QString &address)
{
    SSheetAddress addr = SSheetAddress::fromString(address);
    return cell(addr.row, addr.column);
}

SSheetCell *SSheetWorksheet::cell(quint32 row, quint32 column)
{
    if (row > mLastRow)
        mLastRow = row;

    if (column > mLastColumn)
        mLastColumn = column;

    QString address = SSheetAddress(row, column).toString();
    SSheetCell *cell = mCellHash[address];
    if (!cell) {
        cell = new SSheetCell(this, row, column);
        mCellHash[address] = cell;
    }

    return cell;
}

int SSheetWorksheet::width(quint32 fromColumn, quint32 toColumn)
{
    int _width(0);
    for (quint32 column=fromColumn; column<=toColumn; column++) {
        _width += columnWidth(column);
    }

    return _width;
}

int SSheetWorksheet::width()
{
    return width(0, mLastColumn);
}

int SSheetWorksheet::height(quint32 fromRow, quint32 toRow)
{
    int _height(0);
    for (quint32 row=fromRow; row<=toRow; row++) {
        _height += rowHeight(row);
    }

    return _height;
}

int SSheetWorksheet::height()
{
    return width(0, mLastRow);
}

void SSheetWorksheet::addDrawing(const QList<SSheetDrawing> &drawingList)
{
    mDrawingList.append(drawingList);
}

void SSheetWorksheet::addDrawing(const SSheetDrawing &drawing)
{
    mDrawingList << drawing;
}

QString SSheetWorksheet::toHtml()
{
    QString html;
    QXmlStreamWriter stream(&html);
    stream.setAutoFormatting(true);

    stream.writeStartElement("html");
    stream.writeAttribute("xmlns:v", "urn:schemas-microsoft-com:vml");
    stream.writeAttribute("xmlns:o", "urn:schemas-microsoft-com:office:office");
    stream.writeAttribute("xmlns:x", "urn:schemas-microsoft-com:office:excel");
    stream.writeAttribute("xmlns", "http://www.w3.org/TR/REC-html40");

    stream.writeStartElement("head");
    stream.writeStartElement("meta");
    stream.writeAttribute("http-equiv", "Content-Type");
    stream.writeAttribute("content", "text/html; charset=utf-8");
    stream.writeEndElement();

    stream.writeStartElement("meta");
    stream.writeAttribute("name", "Generator");
    stream.writeAttribute("content", "MPN-Info");
    stream.writeEndElement();

    stream.writeStartElement("style");
    stream.writeCharacters("$style");
    stream.writeEndElement();
    stream.writeEndElement();

    stream.writeStartElement("body");
    stream.writeStartElement("table");

    for (quint32 row=0; row <= lastRow() + 1; row++) {
        stream.writeStartElement("tr");

        for (quint32 column=0; column <= lastColumn() + 1; column++) {
            QVariant data = cell(row, column)->value();

            stream.writeStartElement("td");

            switch ((QMetaType::Type) data.type()) {
            case QMetaType::QString:
                stream.writeAttribute("class", "x0");
                stream.writeAttribute("sdnum", "1033;0;@");
                stream.writeCharacters(data.toString());
                break;
            case QMetaType::Double:
                stream.writeAttribute("class", "x1");
                stream.writeAttribute("sdval", data.toString());
                stream.writeAttribute("sdnum", "1033;0;#,###.00");
                stream.writeCharacters(QString::number(data.toDouble(), 'f'));
                break;
            case QMetaType::Float:
                stream.writeAttribute("class", "x2");
                stream.writeAttribute("sdval", data.toString());
                stream.writeAttribute("sdnum", "1033;0;0.00%");
                stream.writeCharacters(QString::number(data.toDouble() * 100, 'f') + "%");
                break;
            case QMetaType::QDate:
                stream.writeAttribute("class", "x3");
                stream.writeAttribute("sdnum", "1033;0;DD-MM-YYYY");
                stream.writeCharacters(data.toDate().toString("dd-MM-yyyy"));
                break;
            default:
                stream.writeCharacters(data.toString());
                break;
            }

            stream.writeEndElement();
        }

        stream.writeEndElement();
    }

    stream.writeEndElement();
    stream.writeEndElement();

    stream.writeEndElement();

    QString style = "<!--"
                    "table {mso-displayed-decimal-separator:\"\\.\"; mso-displayed-thousand-separator:\"\\,\";}\n"
                    "tr {mso-height-source:auto;}\n"
                    "col {mso-width-source:auto;}\n"
                    "br {mso-data-placement:same-cell;}"
                    "td"
                    "{padding-top:1px;"
                    "padding-right:1px;"
                    "padding-left:1px;"
                    "mso-ignore:padding;"
                    "color:black;"
                    "font-size:11.0pt;"
                    "font-weight:400;"
                    "font-style:normal;"
                    "text-decoration:none;"
                    "font-family:Calibri, sans-serif;"
                    "mso-font-charset:1;"
                    "mso-number-format:General;"
                    "text-align:general;"
                    "vertical-align:bottom;"
                    "border:none;"
                    "mso-background-source:auto;"
                    "mso-pattern:auto;"
                    "mso-protection:locked visible;"
                    "white-space:nowrap;"
                    "mso-rotate:0;}"
                    ".x0 {mso-number-format:\"\\@\";}"
                    ".x1 {mso-number-format:Standard;}"
                    ".x2 {mso-number-format:Percent;}"
                    ".x3 {mso-number-format:\"dd\\\\-mm\\\\-yyyy\\;\\@\";}"
                    "-->";

    html.replace("$style", "\n" + style + "\n");

    return html;

}

void SSheetWorksheet::render(QPainter *painter, const QString &from, const QString &to)
{
    SSheetAddress fromAddr = SSheetAddress::fromString(from);
    SSheetAddress toAddr = SSheetAddress::fromString(to);
    render(painter, fromAddr.row, fromAddr.column, toAddr.row, toAddr.column);
}

void SSheetWorksheet::render(QPainter *painter, quint32 fromRow, quint32 fromColumn, quint32 toRow, quint32 toColumn)
{
    if (!painter)
        return;

    painter->save();

    int height = 0;
    for (quint32 row=fromRow; row<=toRow; row++)
        height += rowHeight(row);

    int width = 0;
    for (quint32 column=fromColumn; column<=toColumn; column++)
        width += columnWidth(column);

    painter->setClipRect(0, 0, width, height);

    for (quint32 row=fromRow; row<=toRow; row++) {
        painter->save();
        for (quint32 column=fromColumn; column<=toColumn; column++) {
            QString address = SSheetAddress(row, column).toString();
            SSheetCell *c = cell(address);

            if (c && !c->isHidden())
                c->render(painter);

            for(int i=0; i<mDrawingList.size(); i++) {
                SSheetDrawing &drawing = mDrawingList[i];
                if (drawing.address.row == row && drawing.address.column == column)
                    drawing.render(painter);
            }

            int width = columnWidth(column);
            painter->translate(width, 0);
        }
        painter->restore();

        int height = rowHeight(row);
        painter->translate(0, height);
    }

    painter->restore();
}

void SSheetWorksheet::render(QPainter *painter)
{
    render(painter, 0, 0, mLastRow, mLastColumn);
}
