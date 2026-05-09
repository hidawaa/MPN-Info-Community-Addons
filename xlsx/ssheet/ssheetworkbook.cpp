#include "ssheetworkbook.h"

#include <QtMath>
#include <QFile>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include <quazip.h>
#include <quazipfile.h>

#include "ssheetworksheet.h"
#include "ssheetcell.h"
#include "ssheetdrawing.h"

SSheetWorkbook::SSheetWorkbook() {}

SSheetWorkbook::~SSheetWorkbook()
{
    close();
}

bool SSheetWorkbook::load(const QString &filename)
{
    QString xmlSharedString;
    QString xmlStyles;
    QStringList xmlDrawingList;
    QString xmlWorkbook;
    QStringList xmlWorksheetList;
    QStringList sharedStringList;
    QList<QFont> fontList;
    QList<SSheetBorder> borderList;
    QList<SSheetStyle> styleList;
    QList<QVariantMap> worksheetMapList;
    QHash<QString, QList<SSheetDrawing>> drawingHash;

    QuaZip zip(filename);
    if (!zip.open(QuaZip::mdUnzip))
        return false;

    QuaZipFile file(&zip);
    if (!zip.setCurrentFile("[Content_Types].xml"))
        return false;

    {
        file.open(QIODevice::ReadOnly);
        QXmlStreamReader xmlReader(&file);
        while (!xmlReader.atEnd()) {
            auto token = xmlReader.readNext();
            if (token != 4)
                continue;

            if (xmlReader.name() != "Override")
                continue;

            auto attributes = xmlReader.attributes();
            QString ContentType = attributes.value("ContentType").toString();
            QString PartName = attributes.value("PartName").toString();

            // adjusting to quazip path
            if (PartName.left(1) == "/")
                PartName.remove(0, 1);

            if (ContentType == "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml")
                xmlWorkbook = PartName;
            else if (ContentType == "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml")
                xmlWorksheetList << PartName;
            else if (ContentType == "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml")
                xmlSharedString = PartName;
            else if (ContentType == "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml")
                xmlStyles = PartName;
            else if (ContentType == "application/vnd.openxmlformats-officedocument.drawing+xml")
                xmlDrawingList << PartName;
        }
        file.close();
    }

    if (!xmlStyles.isEmpty()) {
        if (!zip.setCurrentFile(xmlStyles))
            return false;

        {
            QStringList xmlPathList;
            file.open(QIODevice::ReadOnly);
            QXmlStreamReader xmlReader(&file);
            while (!xmlReader.atEnd()) {
                auto token = xmlReader.readNext();
                QString name = xmlReader.name().toString();
                if (token == 4)
                    xmlPathList << name;

                QString xmlPath = xmlPathList.join("/");

                if (token == 5)
                    xmlPathList.removeLast();

                if (token != 4)
                    continue;

                if (xmlPath == "styleSheet/fonts/font") {
                    QFont font;

                    do {
                        auto _token = xmlReader.readNext();
                        name = xmlReader.name().toString();
                        if (_token == 4)
                            xmlPathList << name;

                        xmlPath = xmlPathList.join("/");

                        if (_token == 5)
                            xmlPathList.removeLast();

                        if (xmlPath == "styleSheet/fonts/font" && _token == 5)
                            break;

                        if (_token != 4)
                            continue;

                        auto attributes = xmlReader.attributes();

                        if (name == "b")
                            font.setBold(true);
                        else if (name == "i")
                            font.setItalic(true);
                        else if (name == "u")
                            font.setUnderline(true);
                        else if (name == "name")
                            font.setFamily(attributes.value("val").toString());
                        else if (name == "sz")
                            font.setPointSizeF(attributes.value("val").toDouble());
                    } while (true);

                    fontList << font;
                }
                else if (xmlPath == "styleSheet/borders/border") {
                    SSheetBorder border;

                    do {
                        auto _token = xmlReader.readNext();
                        name = xmlReader.name().toString();
                        if (_token == 4)
                            xmlPathList << name;

                        xmlPath = xmlPathList.join("/");

                        if (_token == 5)
                            xmlPathList.removeLast();

                        if (xmlPath == "styleSheet/borders/border" && _token == 5)
                            break;

                        if (_token != 4)
                            continue;

                        auto attributes = xmlReader.attributes();
                        QString type = attributes.value("style").toString();
                        if (type.isEmpty())
                            continue;

                        SSheetBorder::Type borderType(SSheetBorder::None);

                        if (type == "thin")
                            borderType = SSheetBorder::Thin;
                        else if (type == "medium")
                            borderType = SSheetBorder::Medium;
                        else if (type == "thick")
                            borderType = SSheetBorder::Thick;
                        else if (type == "dot")
                            borderType = SSheetBorder::Dot;
                        else if (type == "dash")
                            borderType = SSheetBorder::Dash;
                        else if (type == "dashDot")
                            borderType = SSheetBorder::DashDot;
                        else if (type == "dashDotDot")
                            borderType = SSheetBorder::DashDotDot;

                        if (name == "top")
                            border.top = borderType;
                        else if (name == "bottom")
                            border.bottom = borderType;
                        else if (name == "left")
                            border.left = borderType;
                        else if (name == "right")
                            border.right = borderType;
                    } while (true);

                    borderList << border;
                }
                else if (xmlPath == "styleSheet/cellXfs/xf") {
                    SSheetStyle style;

                    auto xfAttributes = xmlReader.attributes();
                    int borderIndex = xfAttributes.value("borderId").toInt();
                    int fontIndex = xfAttributes.value("fontId").toInt();
                    style.border = borderList[borderIndex];
                    style.font = fontList[fontIndex];

                    do {
                        auto _token = xmlReader.readNext();
                        name = xmlReader.name().toString();
                        if (_token == 4)
                            xmlPathList << name;

                        xmlPath = xmlPathList.join("/");

                        if (_token == 5)
                            xmlPathList.removeLast();

                        if (xmlPath == "styleSheet/cellXfs/xf" && _token == 5)
                            break;

                        if (_token != 4)
                            continue;

                        if (name == "alignment") {
                            auto alignmentAttributes = xmlReader.attributes();
                            int warpText = alignmentAttributes.value("wrapText").toInt();
                            QString horizontalAlign = alignmentAttributes.value("horizontal").toString();
                            QString verticalAlign = alignmentAttributes.value("vertical").toString();

                            if (warpText == 1)
                                style.wrapText = true;

                            if (!horizontalAlign.isEmpty()) {
                                if (horizontalAlign == "left")
                                    style.horizontalAlignment = Qt::AlignLeft;
                                else if (horizontalAlign == "right")
                                    style.horizontalAlignment = Qt::AlignRight;
                                else if (horizontalAlign == "center")
                                    style.horizontalAlignment = Qt::AlignHCenter;
                                else if (horizontalAlign == "justify")
                                    style.horizontalAlignment = Qt::AlignJustify;
                            }

                            if (!verticalAlign.isEmpty()) {
                                if (verticalAlign == "top")
                                    style.verticalAlignment = Qt::AlignTop;
                                else if (verticalAlign == "center")
                                    style.verticalAlignment = Qt::AlignVCenter;
                                else if (verticalAlign == "bottom")
                                    style.verticalAlignment = Qt::AlignBottom;
                            }
                        }

                    } while (true);

                    styleList << style;
                }
            }
            file.close();
        }
    }

    if (!xmlSharedString.isEmpty()) {
        if (!zip.setCurrentFile(xmlSharedString))
            return false;

        {
            QStringList xmlPathList;
            file.open(QIODevice::ReadOnly);
            QXmlStreamReader xmlReader(&file);
            while (!xmlReader.atEnd()) {
                auto token = xmlReader.readNext();
                QString name = xmlReader.name().toString();
                if (token == 4)
                    xmlPathList << name;

                QString xmlPath = xmlPathList.join("/");

                if (token == 5)
                    xmlPathList.removeLast();

                if (token != 4 && token != 6)
                    continue;

                if (xmlPath == "sst/si" && token == 4) {
                    QString text;

                    do {
                        auto _token = xmlReader.readNext();
                        name = xmlReader.name().toString();
                        if (_token == 4)
                            xmlPathList << name;

                        xmlPath = xmlPathList.join("/");

                        if (_token == 5)
                            xmlPathList.removeLast();

                        if (xmlPath == "sst/si" && _token == 5)
                            break;

                        if (_token != 4 && _token != 6)
                            continue;

                        if (_token == 6) {
                            if (xmlPath == "sst/si/t")
                                text = xmlReader.text().toString();
                            else if (xmlPath == "sst/si/r/t")
                                text.append(xmlReader.text().toString());
                        }

                    } while (true);


                    sharedStringList << text;
                }
            }
            file.close();
        }
    }

    foreach (const QString &xmlDrawing, xmlDrawingList) {
        QHash<QString, QString> relHash;
        QString relFilename = QString("xl/drawings/_rels/%1.rels").arg(xmlDrawing.section("/", -1));
        if (zip.setCurrentFile(relFilename)) {
            QStringList xmlPathList;
            file.open(QIODevice::ReadOnly);
            QXmlStreamReader xmlReader(&file);
            while (!xmlReader.atEnd()) {
                auto token = xmlReader.readNext();
                QString name = xmlReader.name().toString();
                if (token == 4)
                    xmlPathList << name;

                QString xmlPath = xmlPathList.join("/");

                if (token == 5)
                    xmlPathList.removeLast();

                if (token != 4)
                    continue;

                if (xmlPath == "Relationships/Relationship") {
                    auto attributes = xmlReader.attributes();
                    QString id = attributes.value("Id").toString();
                    QString target = attributes.value("Target").toString().replace("../", "xl/");
                    relHash[id] = target;
                }
            }
            file.close();
        }

        QHash<QString, QImage> relImageHash;
        QHashIterator<QString, QString> relIterator(relHash);
        while (relIterator.hasNext()) {
            relIterator.next();

            if (zip.setCurrentFile(relIterator.value())) {
                file.open(QIODevice::ReadOnly);
                relImageHash[relIterator.key()] = QImage::fromData(file.readAll());
                file.close();
            }
        }

        if (!zip.setCurrentFile(xmlDrawing))
            return false;

        {
            QList<SSheetDrawing> drawingList;
            QStringList xmlPathList;
            file.open(QIODevice::ReadOnly);
            QXmlStreamReader xmlReader(&file);
            while (!xmlReader.atEnd()) {
                auto token = xmlReader.readNext();
                QString name = xmlReader.name().toString();
                if (token == 4)
                    xmlPathList << name;

                QString xmlPath = xmlPathList.join("/");

                if (token == 5)
                    xmlPathList.removeLast();

                if (token != 4)
                    continue;

                if (xmlPath == "wsDr/twoCellAnchor") {
                    SSheetDrawing drawing;

                    do {
                        auto _token = xmlReader.readNext();
                        name = xmlReader.name().toString();
                        if (_token == 4)
                            xmlPathList << name;

                        xmlPath = xmlPathList.join("/");

                        if (_token == 5)
                            xmlPathList.removeLast();

                        if (xmlPath == "wsDr/twoCellAnchor" && _token == 5)
                            break;

                        if (_token != 4 && _token != 6)
                            continue;

                        if (_token == 4) {
                            if (xmlPath == "wsDr/twoCellAnchor/pic/blipFill/blip") {
                                auto attributes = xmlReader.attributes();
                                drawing.image = relImageHash[attributes.value("r:embed").toString()];
                            }
                            else if (xmlPath == "wsDr/twoCellAnchor/pic/spPr/xfrm/ext") {
                                auto attributes = xmlReader.attributes();
                                drawing.size.setWidth(qRound(attributes.value("cx").toDouble() / 9525));
                                drawing.size.setHeight(qRound(attributes.value("cy").toDouble() / 9525));
                            }
                        }
                        else if (_token == 6) {
                            if (xmlPath == "wsDr/twoCellAnchor/from/col")
                                drawing.address.column = xmlReader.text().toUInt();
                            else if (xmlPath == "wsDr/twoCellAnchor/from/row")
                                drawing.address.row = xmlReader.text().toUInt();
                            else if (xmlPath == "wsDr/twoCellAnchor/from/colOff")
                                drawing.offset.setX(qRound(xmlReader.text().toDouble() / 9525));
                            else if (xmlPath == "wsDr/twoCellAnchor/from/rowOff")
                                drawing.offset.setY(qRound(xmlReader.text().toDouble() / 9525));
                        }

                    } while (true);

                    drawingList << drawing;
                }
            }
            file.close();

            drawingHash[xmlDrawing] = drawingList;
        }
    }

    if (!xmlWorkbook.isEmpty()) {
        if (!zip.setCurrentFile(xmlWorkbook))
            return false;

        {
            QStringList xmlPathList;
            file.open(QIODevice::ReadOnly);
            QXmlStreamReader xmlReader(&file);
            while (!xmlReader.atEnd()) {
                auto token = xmlReader.readNext();
                QString name = xmlReader.name().toString();
                if (token == 4)
                    xmlPathList << name;

                QString xmlPath = xmlPathList.join("/");

                if (token == 5)
                    xmlPathList.removeLast();

                if (token != 4)
                    continue;

                if (xmlPath == "workbook/sheets/sheet") {
                    auto attributes = xmlReader.attributes();

                    QVariantMap worksheetMap;
                    worksheetMap["index"] = attributes.value("r:id").toString().remove("rId").toInt() - 1;
                    worksheetMap["name"] = attributes.value("name").toString();
                    worksheetMapList << worksheetMap;
                }
            }
            file.close();
        }
    }

    foreach (const QVariantMap &worksheetMap, worksheetMapList) {
        int sheetIndex = worksheetMap["index"].toInt();
        QString sheetname = worksheetMap["name"].toString();
        QString sheetFilename = xmlWorksheetList[sheetIndex];

        QHash<QString, QString> relHash;
        QString relFilename = QString("xl/worksheets/_rels/%1.rels").arg(sheetFilename.section("/", -1));
        if (zip.setCurrentFile(relFilename)) {
            QStringList xmlPathList;
            file.open(QIODevice::ReadOnly);
            QXmlStreamReader xmlReader(&file);
            while (!xmlReader.atEnd()) {
                auto token = xmlReader.readNext();
                QString name = xmlReader.name().toString();
                if (token == 4)
                    xmlPathList << name;

                QString xmlPath = xmlPathList.join("/");

                if (token == 5)
                    xmlPathList.removeLast();

                if (token != 4)
                    continue;

                if (xmlPath == "Relationships/Relationship") {
                    auto attributes = xmlReader.attributes();
                    QString id = attributes.value("Id").toString();
                    QString target = attributes.value("Target").toString().replace("../", "xl/");

                    relHash[id] = target;
                }
            }
            file.close();
        }

        if (!zip.setCurrentFile(sheetFilename))
            continue;

        SSheetWorksheet *worksheet = createWorksheet(sheetname);

        QStringList xmlPathList;
        file.open(QIODevice::ReadOnly);
        QXmlStreamReader xmlReader(&file);
        while (!xmlReader.atEnd()) {
            auto token = xmlReader.readNext();
            QString name = xmlReader.name().toString();
            if (token == 4)
                xmlPathList << name;

            QString xmlPath = xmlPathList.join("/");

            if (token == 5)
                xmlPathList.removeLast();

            if (token != 4)
                continue;

            if (xmlPath == "worksheet/sheetFormatPr") {
                auto attributes = xmlReader.attributes();
                worksheet->setDefaultRowHeight(qRound(attributes.value("defaultRowHeight").toFloat() * 4 / 3));
                worksheet->setDefaultColumnWidth(qRound(attributes.value("defaultColWidth").toFloat() * 7.00027344818157f));
            }
            else if (xmlPath == "worksheet/cols/col") {
                auto attributes = xmlReader.attributes();
                int custom = attributes.value("customWidth").toInt();
                if (custom == 1) {
                    uint min = attributes.value("min").toUInt();
                    uint max = attributes.value("max").toUInt();
                    int width = qRound(attributes.value("width").toDouble() * 7.00027344818157);
                    for (uint c=min; c<=max; c++)
                        worksheet->setColumnWidth(c - 1, width);
                }
            }
            else if (xmlPath == "worksheet/sheetData/row") {
                auto attributes = xmlReader.attributes();
                int custom = attributes.value("customHeight").toInt();
                if (custom == 1) {
                    uint r = attributes.value("r").toUInt();
                    int height = qRound(attributes.value("ht").toDouble() * 4 / 3);
                    worksheet->setRowHeight(r - 1, height);
                }
            }
            else if (xmlPath == "worksheet/sheetData/row/c") {
                auto attributes = xmlReader.attributes();
                QString address = attributes.value("r").toString();
                QString type = attributes.value("t").toString();
                int styleIndex = attributes.value("s").toInt();

                SSheetCell *cell = worksheet->cell(address);
                cell->setStyle(styleList[styleIndex]);

                do {
                    auto _token = xmlReader.readNext();
                    name = xmlReader.name().toString();
                    if (_token == 4)
                        xmlPathList << name;

                    xmlPath = xmlPathList.join("/");

                    if (_token == 5)
                        xmlPathList.removeLast();

                    if (xmlPath == "worksheet/sheetData/row/c" && _token == 5)
                        break;

                    if (xmlPath == "worksheet/sheetData/row/c/v" && _token == 6) {
                        QString text = xmlReader.text().toString();
                        if (type == "s")
                            cell->setValue(sharedStringList[text.toInt()]);
                        else
                            cell->setValue(text.toDouble());
                    }

                } while (true);
            }
            else if (xmlPath == "worksheet/mergeCells/mergeCell") {
                auto attributes = xmlReader.attributes();
                QStringList rangeList = attributes.value("ref").toString().split(":");
                SSheetAddress fromAddr = SSheetAddress::fromString(rangeList[0]);
                SSheetAddress toAddr = SSheetAddress::fromString(rangeList[1]);

                int rowSpan = int(toAddr.row - fromAddr.row) + 1;
                int columnSpan = int(toAddr.column - fromAddr.column) + 1;

                SSheetCell *cell = worksheet->cell(rangeList[0]);
                cell->setRowSpan(rowSpan);
                cell->setColumnSpan(columnSpan);
            }
            else if (xmlPath == "worksheet/drawing") {
                auto attributes = xmlReader.attributes();
                worksheet->addDrawing(drawingHash[relHash[attributes.value("r:id").toString()]]);
            }
        }

        file.close();
    }

    return true;
}

bool SSheetWorkbook::save(const QString &filename)
{
    QDateTime dateTime = QDateTime::currentDateTime();

    QuaZip zip(filename);
    if (!zip.open(QuaZip::mdCreate))
        return false;

    QuaZipFile file(&zip);

    quint32 numString = 0;
    QStringList sharedStringList;
    bool useCurrency = false;
    bool useDateFormat = false;

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("_rels/.rels")))
    {
        QXmlStreamWriter relRootStream(&file);
        relRootStream.setAutoFormatting(true);
        relRootStream.writeStartDocument();

        relRootStream.writeStartElement("Relationships");
        relRootStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/package/2006/relationships");

        relRootStream.writeStartElement("Relationship");
        relRootStream.writeAttribute("Id", "rId3");
        relRootStream.writeAttribute("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties");
        relRootStream.writeAttribute("Target", "docProps/app.xml");
        relRootStream.writeEndElement();

        relRootStream.writeStartElement("Relationship");
        relRootStream.writeAttribute("Id", "rId2");
        relRootStream.writeAttribute("Type", "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties");
        relRootStream.writeAttribute("Target", "docProps/core.xml");
        relRootStream.writeEndElement();

        relRootStream.writeStartElement("Relationship");
        relRootStream.writeAttribute("Id", "rId1");
        relRootStream.writeAttribute("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
        relRootStream.writeAttribute("Target", "xl/workbook.xml");
        relRootStream.writeEndElement();

        relRootStream.writeEndElement();
        relRootStream.writeEndDocument();

        file.close();
    }

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("docProps/app.xml")))
    {
        QXmlStreamWriter appStream(&file);
        appStream.setAutoFormatting(true);
        appStream.writeStartDocument();
        appStream.writeStartElement("Properties");
        appStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties");
        appStream.writeAttribute("xmlns:vt", "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes");

        appStream.writeTextElement("Application", "Microsoft Excel");
        appStream.writeTextElement("DocSecurity", "0");
        appStream.writeTextElement("ScaleCrop", "false");

        appStream.writeStartElement("HeadingPairs");
        appStream.writeStartElement("vt:vector");
        appStream.writeAttribute("size", "2");
        appStream.writeAttribute("baseType", "variant");

        appStream.writeStartElement("vt:variant");
        appStream.writeTextElement("vt:lpstr", "Worksheets");
        appStream.writeEndElement();

        appStream.writeStartElement("vt:variant");
        appStream.writeTextElement("vt:i4", QString::number(mWorksheetList.size()));
        appStream.writeEndElement();

        appStream.writeEndElement();
        appStream.writeEndElement();

        appStream.writeStartElement("TitlesOfParts");
        appStream.writeStartElement("vt:vector");
        appStream.writeAttribute("size", QString::number(mWorksheetList.size()));
        appStream.writeAttribute("baseType", "lpstr");
        foreach (SSheetWorksheet *ws, mWorksheetList)
            appStream.writeTextElement("vt:lpstr", ws->name());
        appStream.writeEndElement();
        appStream.writeEndElement();

        appStream.writeTextElement("LinksUpToDate", "false");
        appStream.writeTextElement("SharedDoc", "false");
        appStream.writeTextElement("HyperlinksChanged", "false");
        appStream.writeTextElement("AppVersion", "14.0300");

        appStream.writeEndElement();
        appStream.writeEndDocument();

        file.close();
    }

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("docProps/core.xml")))
    {
        QXmlStreamWriter coreStream(&file);
        coreStream.setAutoFormatting(true);
        coreStream.writeStartDocument();

        coreStream.writeStartElement("cp:coreProperties");
        coreStream.writeAttribute("xmlns:cp", "http://schemas.openxmlformats.org/package/2006/metadata/core-properties");
        coreStream.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
        coreStream.writeAttribute("xmlns:dcterms", "http://purl.org/dc/terms/");
        coreStream.writeAttribute("xmlns:dcmitype", "http://purl.org/dc/dcmitype/");
        coreStream.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");

        coreStream.writeTextElement("dc:creator", "MPN-Info");
        coreStream.writeTextElement("cp:lastModifiedBy", "MPN-Info");

        coreStream.writeStartElement("dcterms:created");
        coreStream.writeAttribute("xsi:type", "dcterms:W3CDTF");
        coreStream.writeCharacters(dateTime.toString("yyyy-MM-ddTHH:mm:ssZ"));
        coreStream.writeEndElement();

        coreStream.writeStartElement("dcterms:modified");
        coreStream.writeAttribute("xsi:type", "dcterms:W3CDTF");
        coreStream.writeCharacters(dateTime.toString("yyyy-MM-ddTHH:mm:ssZ"));
        coreStream.writeEndElement();

        coreStream.writeEndElement();
        coreStream.writeEndDocument();

        file.close();
    }

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("xl/_rels/workbook.xml.rels")))
    {
        QXmlStreamWriter relXlStream(&file);
        relXlStream.setAutoFormatting(true);
        relXlStream.writeStartDocument();

        relXlStream.writeStartElement("Relationships");
        relXlStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/package/2006/relationships");

        int id = 0;
        for (int i=0; i < mWorksheetList.size(); i++) {
            id++;
            relXlStream.writeStartElement("Relationship");
            relXlStream.writeAttribute("Id", QString("rId%1").arg(id));
            relXlStream.writeAttribute("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet");
            relXlStream.writeAttribute("Target", QString("worksheets/sheet%1.xml").arg(id));
            relXlStream.writeEndElement();
        }

        relXlStream.writeStartElement("Relationship");
        relXlStream.writeAttribute("Id", QString("rId%1").arg(++id));
        relXlStream.writeAttribute("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles");
        relXlStream.writeAttribute("Target", "styles.xml");
        relXlStream.writeEndElement();

        relXlStream.writeStartElement("Relationship");
        relXlStream.writeAttribute("Id", QString("rId%1").arg(++id));
        relXlStream.writeAttribute("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings");
        relXlStream.writeAttribute("Target", "sharedStrings.xml");
        relXlStream.writeEndElement();

        relXlStream.writeEndElement();
        relXlStream.writeEndDocument();

        file.close();
    }

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("xl/workbook.xml")))
    {
        QXmlStreamWriter workbookStream(&file);
        workbookStream.setAutoFormatting(true);
        workbookStream.writeStartDocument();

        workbookStream.writeStartElement("workbook");
        workbookStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/spreadsheetml/2006/main");
        workbookStream.writeAttribute("xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships");

        /*
        workbookStream.writeStartElement("fileVersion");
        workbookStream.writeAttribute("appName", "xl");
        workbookStream.writeAttribute("lastEdited", "5");
        workbookStream.writeAttribute("lowestEdited", "5");
        workbookStream.writeAttribute("rupBuild", "9302");
        workbookStream.writeEndElement();

        workbookStream.writeStartElement("workbookPr");
        workbookStream.writeAttribute("defaultThemeVersion", "124226");
        workbookStream.writeEndElement();
        */

        workbookStream.writeStartElement("bookViews");
        workbookStream.writeStartElement("workbookView ");
        workbookStream.writeAttribute("xWindow", "0");
        workbookStream.writeAttribute("yWindow", "0");
        workbookStream.writeEndElement();
        workbookStream.writeEndElement();

        workbookStream.writeStartElement("sheets");
        for (int i=0; i < mWorksheetList.size(); i++) {
            workbookStream.writeStartElement("sheet");
            workbookStream.writeAttribute("name", mWorksheetList[i]->name());
            workbookStream.writeAttribute("sheetId", QString::number(i+1));
            workbookStream.writeAttribute("r:id", QString("rId%1").arg(i+1));
            workbookStream.writeEndElement();
        }
        workbookStream.writeEndElement();

        workbookStream.writeEndElement();
        workbookStream.writeEndDocument();

        file.close();
    }

    for (int i=0; i < mWorksheetList.size(); i++)
    {
        QString sfilename = QString("sheet%1.xml").arg(i+1);

        SSheetWorksheet *ws = mWorksheetList[i];
        if (file.open(QIODevice::WriteOnly, QString("xl/worksheets/%1").arg(sfilename)))
        {
            QXmlStreamWriter worksheetStream(&file);
            worksheetStream.setAutoFormatting(true);
            worksheetStream.writeStartDocument();

            worksheetStream.writeStartElement("worksheet");
            worksheetStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/spreadsheetml/2006/main");
            worksheetStream.writeAttribute("xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships");
            worksheetStream.writeAttribute("xmlns:mc", "http://schemas.openxmlformats.org/markup-compatibility/2006");
            worksheetStream.writeAttribute("xmlns:x14ac", "http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac");

            worksheetStream.writeStartElement("sheetViews");
            worksheetStream.writeStartElement("sheetView");
            if (i == 0)
                worksheetStream.writeAttribute("tabSelected", "1");
            worksheetStream.writeAttribute("workbookViewId", "0");
            worksheetStream.writeEndElement();
            worksheetStream.writeEndElement();

            worksheetStream.writeStartElement("sheetData");
            for (quint32 row=0; row <= ws->lastRow(); row++) {

                worksheetStream.writeStartElement("row");
                worksheetStream.writeAttribute("r", QString::number(row + 1));

                for (quint32 column=0; column <= ws->lastColumn(); column++) {
                    SSheetCell *cell =  ws->cell(row, column);
                    if (!cell)
                        continue;

                    const QVariant &data = cell->value();
                    if (!data.isValid())
                        continue;

                    worksheetStream.writeStartElement("c");
                    worksheetStream.writeAttribute("r", SSheetAddress(row, column).toString());

                    switch (data.type()) {
                    case QVariant::String:
                    {
                        worksheetStream.writeAttribute("t", "s");

                        int index = sharedStringList.indexOf(data.toString());
                        if (index == -1) {
                            worksheetStream.writeTextElement("v", QString::number(sharedStringList.count()));
                            sharedStringList << data.toString();
                        }
                        else {
                            worksheetStream.writeTextElement("v", QString::number(index));
                        }

                        numString++;
                    }
                        break;
                    case QVariant::Double:
                        worksheetStream.writeAttribute("s", "1");
                        worksheetStream.writeTextElement("v", data.toString());
                        useCurrency = true;
                        break;
                    case QVariant::Date:
                        worksheetStream.writeAttribute("s", "2");
                        worksheetStream.writeTextElement("v", QString::number(data.toDate().toJulianDay() - 2415019));
                        useDateFormat = true;
                        break;
                    default:
                        worksheetStream.writeTextElement("v", data.toString());
                    }
                    worksheetStream.writeEndElement();
                }
                worksheetStream.writeEndElement();
            }
            worksheetStream.writeEndElement();

            worksheetStream.writeEndElement();
            worksheetStream.writeEndDocument();

            file.close();
        }
    }

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("xl/styles.xml")))
    {
        QXmlStreamWriter styleStream(&file);
        styleStream.setAutoFormatting(true);
        styleStream.writeStartDocument();

        styleStream.writeStartElement("styleSheet");
        styleStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/spreadsheetml/2006/main");
        styleStream.writeAttribute("xmlns:mc", "http://schemas.openxmlformats.org/markup-compatibility/2006");
        styleStream.writeAttribute("xmlns:x14ac", "http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac");

        styleStream.writeStartElement("numFmts");
        styleStream.writeStartElement("numFmt");
        styleStream.writeAttribute("numFmtId", "165");
        styleStream.writeAttribute("formatCode", "dd/mm/yyyy");
        styleStream.writeEndElement();
        styleStream.writeEndElement();

        styleStream.writeStartElement("fonts");
        styleStream.writeAttribute("count", "1");

        styleStream.writeStartElement("font");

        styleStream.writeStartElement("sz");
        styleStream.writeAttribute("val", "11");
        styleStream.writeEndElement();

        styleStream.writeStartElement("color");
        styleStream.writeAttribute("theme", "1");
        styleStream.writeEndElement();

        styleStream.writeStartElement("name");
        styleStream.writeAttribute("val", "Calibri");
        styleStream.writeEndElement();

        styleStream.writeStartElement("family");
        styleStream.writeAttribute("val", "2");
        styleStream.writeEndElement();

        styleStream.writeStartElement("charset");
        styleStream.writeAttribute("val", "1");
        styleStream.writeEndElement();

        styleStream.writeStartElement("scheme");
        styleStream.writeAttribute("val", "minor");
        styleStream.writeEndElement();

        styleStream.writeEndElement();
        styleStream.writeEndElement();

        styleStream.writeStartElement("fills");
        styleStream.writeAttribute("count", "1");

        styleStream.writeStartElement("fill");

        styleStream.writeStartElement("patternFill");
        styleStream.writeAttribute("patternType", "none");
        styleStream.writeEndElement();

        styleStream.writeEndElement();
        styleStream.writeEndElement();

        styleStream.writeStartElement("borders");
        styleStream.writeAttribute("count", "1");

        styleStream.writeStartElement("border");

        styleStream.writeStartElement("left");
        styleStream.writeEndElement();

        styleStream.writeStartElement("right");
        styleStream.writeEndElement();

        styleStream.writeStartElement("top");
        styleStream.writeEndElement();

        styleStream.writeStartElement("bottom");
        styleStream.writeEndElement();

        styleStream.writeStartElement("diagonal");
        styleStream.writeEndElement();

        styleStream.writeEndElement();
        styleStream.writeEndElement();

        styleStream.writeStartElement("cellStyleXfs");
        styleStream.writeAttribute("count", "1");
        styleStream.writeStartElement("xf");
        styleStream.writeAttribute("numFmtId", "0");
        styleStream.writeAttribute("fontId", "0");
        styleStream.writeAttribute("fillId", "0");
        styleStream.writeAttribute("borderId", "0");
        styleStream.writeEndElement();
        styleStream.writeEndElement();

        styleStream.writeStartElement("cellXfs");

        styleStream.writeAttribute("count", "3");
        styleStream.writeStartElement("xf");
        styleStream.writeAttribute("numFmtId", "0");
        styleStream.writeAttribute("fontId", "0");
        styleStream.writeAttribute("fillId", "0");
        styleStream.writeAttribute("borderId", "0");
        styleStream.writeEndElement();

        styleStream.writeStartElement("xf");
        styleStream.writeAttribute("numFmtId", "4");
        styleStream.writeAttribute("fontId", "0");
        styleStream.writeAttribute("fillId", "0");
        styleStream.writeAttribute("borderId", "0");
        styleStream.writeAttribute("applyNumberFormat", QString::number(useCurrency));
        styleStream.writeEndElement();

        styleStream.writeStartElement("xf");
        styleStream.writeAttribute("numFmtId", "165");
        styleStream.writeAttribute("fontId", "0");
        styleStream.writeAttribute("fillId", "0");
        styleStream.writeAttribute("borderId", "0");
        styleStream.writeAttribute("applyNumberFormat", QString::number(useDateFormat));
        styleStream.writeEndElement();

        styleStream.writeEndElement();

        styleStream.writeStartElement("cellStyles");
        styleStream.writeAttribute("count", "1");
        styleStream.writeStartElement("cellStyle");
        styleStream.writeAttribute("name", "Normal");
        styleStream.writeAttribute("xfId", "0");
        styleStream.writeAttribute("builtinId", "0");
        styleStream.writeEndElement();
        styleStream.writeEndElement();

        styleStream.writeStartElement("dxfs");
        styleStream.writeAttribute("count", "0");
        styleStream.writeEndElement();

        styleStream.writeStartElement("tableStyles");
        styleStream.writeAttribute("count", "0");
        styleStream.writeEndElement();

        styleStream.writeEndElement();
        styleStream.writeEndDocument();

        file.close();
    }

    if (sharedStringList.count() > 0)
    {
        if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("xl/sharedStrings.xml")))
        {
            QXmlStreamWriter sharedStream(&file);
            sharedStream.setAutoFormatting(true);
            sharedStream.writeStartDocument();

            sharedStream.writeStartElement("sst");
            sharedStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/spreadsheetml/2006/main");
            sharedStream.writeAttribute("count", QString::number(numString));
            sharedStream.writeAttribute("uniqueCount", QString::number(sharedStringList.count()));

            foreach (const QString &string, sharedStringList) {
                sharedStream.writeStartElement("si");
                sharedStream.writeTextElement("t", string);
                sharedStream.writeEndElement();
            }

            sharedStream.writeEndElement();
            sharedStream.writeEndDocument();

            file.close();
        }
    }

    if (file.open(QIODevice::WriteOnly, QuaZipNewInfo("[Content_Types].xml")))
    {
        QXmlStreamWriter contentTypeStream(&file);
        contentTypeStream.setAutoFormatting(true);
        contentTypeStream.writeStartDocument();

        contentTypeStream.writeStartElement("Types");
        contentTypeStream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/package/2006/content-types");

        contentTypeStream.writeStartElement("Default");
        contentTypeStream.writeAttribute("Extension", "rels");
        contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-package.relationships+xml");
        contentTypeStream.writeEndElement();

        contentTypeStream.writeStartElement("Default");
        contentTypeStream.writeAttribute("Extension", "xml");
        contentTypeStream.writeAttribute("ContentType", "application/xml");
        contentTypeStream.writeEndElement();

        contentTypeStream.writeStartElement("Override");
        contentTypeStream.writeAttribute("PartName", "/xl/workbook.xml");
        contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml");
        contentTypeStream.writeEndElement();

        for (int i=0; i < mWorksheetList.size(); i++) {
            contentTypeStream.writeStartElement("Override");
            contentTypeStream.writeAttribute("PartName", QString("/xl/worksheets/sheet%1.xml").arg(i+1));
            contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml");
            contentTypeStream.writeEndElement();
        }

        if (!sharedStringList.isEmpty()) {
            contentTypeStream.writeStartElement("Override");
            contentTypeStream.writeAttribute("PartName", "/xl/sharedStrings.xml");
            contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml");
            contentTypeStream.writeEndElement();
        }

        contentTypeStream.writeStartElement("Override");
        contentTypeStream.writeAttribute("PartName", "/xl/styles.xml");
        contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml");
        contentTypeStream.writeEndElement();

        contentTypeStream.writeStartElement("Override");
        contentTypeStream.writeAttribute("PartName", "/docProps/core.xml");
        contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-package.core-properties+xml");
        contentTypeStream.writeEndElement();

        contentTypeStream.writeStartElement("Override");
        contentTypeStream.writeAttribute("PartName", "/docProps/app.xml");
        contentTypeStream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.extended-properties+xml");
        contentTypeStream.writeEndElement();

        contentTypeStream.writeEndElement();
        contentTypeStream.writeEndDocument();

        file.close();
    }

    zip.close();

    return true;
}

void SSheetWorkbook::close()
{
    for (int i=0; i<mWorksheetList.size(); i++)
        delete mWorksheetList[i];
    mWorksheetList.clear();
}

SSheetWorksheet *SSheetWorkbook::createWorksheet(const QString &name)
{
    SSheetWorksheet *worksheet = new SSheetWorksheet(this, name);
    mWorksheetList << worksheet;
    return worksheet;
}

SSheetWorksheet *SSheetWorkbook::worksheet(int index)
{
    if (index < 0 || index >= mWorksheetList.size())
        return nullptr;

    return mWorksheetList[index];
}

SSheetWorksheet *SSheetWorkbook::worksheet(const QString &name)
{
    foreach (SSheetWorksheet *ws, mWorksheetList)
        if (ws->name() == name)
            return ws;

    return nullptr;
}

QStringList SSheetWorkbook::worksheetNames()
{
    QStringList list;
    foreach (SSheetWorksheet *ws, mWorksheetList)
        list << ws->name();
    return list;
}

int SSheetWorkbook::countWorksheet()
{
    return mWorksheetList.size();
}
