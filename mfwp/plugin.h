#ifndef PLUGIN_H
#define PLUGIN_H

#include <interface.h>
#include <QObject>

class Plugin : public QObject, Interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IID_INTERFACE FILE "plugin.json")
    Q_INTERFACES(Interface)

public:
    QString author() override { return "System"; }
    QString authorEmail() override { return ""; }
    QString group() override { return "MFWP"; }
    QString version() override { return "1.0.0"; }
    QString description() override { return "Pencarian Wajib Pajak"; }
    QList<AddOnPtr> addOns() override;
};

#endif // PLUGIN_H
