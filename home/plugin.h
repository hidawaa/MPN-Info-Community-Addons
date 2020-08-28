#ifndef PLUGIN_H
#define PLUGIN_H

#include <QWidget>
#include <QtPlugin>

#include "interface.h"

class Plugin : public QObject, Interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IID_INTERFACE FILE "plugin.json")
    Q_INTERFACES(Interface)

public:
    QString author() { return "Ichdyan Thalasa"; }
    QString authorEmail() { return "ichdyan.thalasa@gmail.com"; }
    QString group() { return "Monitoring"; }
    QString version() { return "0.0.1"; }
    QString description() { return "Monitoring Penerimaan Kantor"; }
    QList<AddOnPtr> addOns() { return QList<AddOnPtr>(); }
};

#endif
