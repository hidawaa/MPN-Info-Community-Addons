#ifndef HOMEADDON_H
#define HOMEADDON_H

#include <coreengine.h>
#include <interface.h>

#include "src/homepage.h"

class HomeAddOn : public AddOn
{
public:
    QString name() { return "home_page"; }
    QString group() { return "Home"; }
    QString title() { return "Dashboard"; }
    AddOnTypes type() { return AddOnPage; }
    int loadFlags() { return AddOnCreateMenu | AddOnExecAfterLogin; }
    int permission() { return AddOnAdministrators | AddOnUsers; }
    ObjectPtr newObject() { return nullptr; }
    PagePtr newPage() { return PagePtr(new HomePage(engine)); }
    ProcessPtr newProcess() { return nullptr; }
};

#endif // HOMEADDON_H
