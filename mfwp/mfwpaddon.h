#ifndef MFWPADDON_H
#define MFWPADDON_H

#include <coreengine.h>
#include <interface.h>

#include "src/masterfilesearchpage.h"

class MfwpAddOn : public AddOn
{
public:
    QString name() { return "mfwp_search"; }
    QString group() { return "MFWP"; }
    QString title() { return "Cari Wajib Pajak"; }
    AddOnTypes type() { return AddOnPage; }
    int loadFlags() { return AddOnCreateMenu; }
    int permission() { return AddOnAdministrators | AddOnUsers; }
    ObjectPtr newObject() { return nullptr; }
    PagePtr newPage() { return PagePtr(new MasterfileSearchPage(engine)); }
    ProcessPtr newProcess() { return nullptr; }
};

#endif // MFWPADDON_H
