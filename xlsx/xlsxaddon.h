#ifndef XLSXOBJECT_H
#define XLSXOBJECT_H

#include <QObject>
#include <QFileDialog>
#include <QDebug>

#include <coreengine.h>
#include <ssheetworkbook.h>
#include <ssheetworksheet.h>
#include <ssheetcell.h>

class XlsxObject : public Object
{
public:
    XlsxObject() {
        mWorkbook.reset(new SSheetWorkbook);
    }

    QStringList keys() {
        return QStringList() << "load" << "save" << "createWorksheet" << "setActiveWorksheet" << "setValue" << "value";
    }

    QVariant exec(const QString &key, const QVariant &args = QVariant()) {
        if (key == "load") {
            bool result = mWorkbook->load(args.toString());
            if (!result)
                return false;

            mActiveWorksheet = mWorkbook->worksheet(0);
            return true;
        }
        else if (key == "save") {
            return mWorkbook->save(args.toString());
        }
        else if (key == "createWorksheet") {
            mActiveWorksheet = mWorkbook->createWorksheet(args.toString());
            if (mActiveWorksheet)
                return true;
        }
        else if (key == "setActiveWorksheet") {
            SSheetWorksheet *worksheet(nullptr);
            if (args.type() == QVariant::Int)
                worksheet = mWorkbook->worksheet(args.toInt());
            else if (args.type() == QVariant::String)
                worksheet = mWorkbook->worksheet(args.toString());

            if (!worksheet)
                return false;

            mActiveWorksheet = worksheet;
            return true;
        }
        else if (key == "setValue") {
            if (!mActiveWorksheet)
                return false;

            QVariantMap argsMap = args.toMap();
            uint row = argsMap["row"].toUInt();
            uint column = argsMap["column"].toUInt();
            QString address = argsMap["cell"].toString();

            SSheetCell *cell = address.isEmpty()? mActiveWorksheet->cell(row, column) : mActiveWorksheet->cell(address);
            if (!cell)
                return false;

            cell->setValue(argsMap["value"]);
            return true;
        }
        else if (key == "value") {
            if (!mActiveWorksheet)
                return QString();

            QVariantMap argsMap = args.toMap();
            uint row = argsMap["row"].toUInt();
            uint column = argsMap["column"].toUInt();
            QString address = argsMap["cell"].toString();

            SSheetCell *cell = address.isEmpty()? mActiveWorksheet->cell(row, column) : mActiveWorksheet->cell(address);
            if (!cell)
                return QString();

            return cell->value();
        }

        return false;
    }

private:
    QScopedPointer<SSheetWorkbook> mWorkbook;
    SSheetWorksheet *mActiveWorksheet;
};

class XlsxAddOn : public AddOn
{
    QString name() { return "xlsx"; }
    QString group() { return "Utility"; }
    QString title() { return ""; }
    AddOnTypes type() { return AddOnObject; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ObjectPtr newObject() { return ObjectPtr(new XlsxObject); }
    PagePtr newPage() { return nullptr; }
    ProcessPtr newProcess() { return nullptr; }
};

#endif // XLSXOBJECT_H
