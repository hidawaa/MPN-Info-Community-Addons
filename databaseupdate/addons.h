#ifndef DATABASEUPDATEADDON_H
#define DATABASEUPDATEADDON_H

#include <coreengine.h>
#include <interface.h>

#include "databaseupdate.h"
#include "databaseupdatekantor.h"
#include "databaseupdatemap.h"
#include "databaseupdateklu.h"
#include "databaseupdatejatuhtempo.h"
#include "databaseupdatemaxlapor.h"

class DatabaseUpdateAddOn : public AddOn
{
public:
    QString name() { return "database_update"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return AddOnExecBeforeLogin; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdate(engine)); }
};

class DatabaseUpdateKantorAddOn : public AddOn
{
public:
    QString name() { return "database_update_kantor"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update Kantor"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdateKantor(engine)); }
};

class DatabaseUpdateMapAddOn : public AddOn
{
public:
    QString name() { return "database_update_map"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update Kode MAP"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdateMap(engine)); }
};

class DatabaseUpdateKluAddOn : public AddOn
{
public:
    QString name() { return "database_update_klu"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update KLU"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdateKlu(engine)); }
};

class DatabaseUpdateJatuhTempoAddOn : public AddOn
{
public:
    QString name() { return "database_update_jatuhtempo"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update Jatuh Tempo"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdateJatuhTempo(engine)); }
};


class DatabaseUpdateMaxLaporAddOn : public AddOn
{
public:
    QString name() { return "database_update_maxlapor"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update Max Lapor"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdateMaxLapor(engine)); }
};

#endif // DATABASEUPDATEADDON_H
