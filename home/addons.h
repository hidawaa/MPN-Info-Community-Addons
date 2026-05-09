#ifndef EXAMPLEADDON_H
#define EXAMPLEADDON_H

#include <coreengine.h>
#include <interface.h>

#include "src/homepage.h"

class ExamplePageAddOn : public AddOn
{
public:
    QString name() { return "home_page"; }
    QString group() { return "Monitoring"; }
    QString title() { return "Home"; }
    AddOnTypes type() { return AddOnPage; }
    int loadFlags() { return AddOnLoadCreateMenu | AddOnLoadExecAfterLogin; }
    int permission() { return 0; }
    Object *newObject() { return nullptr; }
    Page *newPage() { return new HomePage(engine); }
    Process *newProcess() { return nullptr; }
};

#endif // EXAMPLEADDON_H
