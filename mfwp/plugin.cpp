#include "plugin.h"
#include "mfwpaddon.h"

QList<AddOnPtr> Plugin::addOns()
{
    QList<AddOnPtr> list;
    list << AddOnPtr(new MfwpAddOn);

    return list;
}
