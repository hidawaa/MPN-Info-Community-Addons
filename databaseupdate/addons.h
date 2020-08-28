#ifndef DATABASEUPDATEADDON_H
#define DATABASEUPDATEADDON_H

#include <coreengine.h>
#include <interface.h>

#include "databaseupdate.h"

class DatabaseUpdateAddOn : public AddOn
{
public:
    QString name() { return "database_update"; }
    QString group() { return "Database"; }
    QString title() { return "Database Update"; }
    AddOnTypes type() { return AddOnProcess; }
    int loadFlags() { return AddOnCreateMenu | AddOnExecBeforeLogin; }
    int permission() { return 0; }
    ObjectPtr newObject() { return ObjectPtr(); }
    PagePtr newPage() { return PagePtr(); }
    ProcessPtr newProcess() { return ProcessPtr(new DatabaseUpdate(engine)); }
};

#endif // DATABASEUPDATEADDON_H
