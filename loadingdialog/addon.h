#ifndef LOADINGDIALOGADDON_H
#define LOADINGDIALOGADDON_H

#include <QObject>
#include <QDebug>

#include <coreengine.h>
#include "loadingdialog.h"

class LoadingDialogObject : public Object
{
public:
    QStringList keys() {
        return QStringList() << "setMessage" << "setProgress" << "exec" << "show";
    }

    QVariant exec(const QString &key, const QVariant &args = QVariant()) {
        if (key == "setMessage") {
            QString message = args.toString();
            mDialog.setMessage(message);
        }

        else if (key == "setProgress") {
            QVariantMap argsMap = args.toMap();
            int value = argsMap["value"].toInt();
            int range = argsMap["range"].toInt();

            mDialog.setProgress(value, range);

        }

        else if (key == "exec") {
            mDialog.exec();
        }

        else if (key == "show") {
            mDialog.show();
        }

        return false;
    }

private:
    LoadingDialog mDialog;
};

class LoadingAddOn : public AddOn
{
    QString name() { return "dialog_loading"; }
    QString group() { return "Dialog"; }
    QString title() { return "Loading Dialog"; }
    AddOnTypes type() { return AddOnObject; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ObjectPtr newObject() { return ObjectPtr(new LoadingDialogObject); }
};

#endif // LOADINGDIALOGADDON_H
