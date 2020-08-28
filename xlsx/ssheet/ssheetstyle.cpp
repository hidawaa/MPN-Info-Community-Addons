#include "ssheetstyle.h"

#include <QtCore/qnamespace.h>

SSheetStyle::SSheetStyle() :
    horizontalAlignment(Qt::AlignLeft),
    verticalAlignment(Qt::AlignBottom),
    wrapText(false)
{
}

SSheetStyle::SSheetStyle(const SSheetStyle &other) :
    font(other.font),
    border(other.border),
    horizontalAlignment(other.horizontalAlignment),
    verticalAlignment(other.verticalAlignment),
    wrapText(other.wrapText) {}

SSheetStyle &SSheetStyle::operator=(const SSheetStyle &other)
{
    font = other.font;
    border = other.border;
    horizontalAlignment = other.horizontalAlignment;
    verticalAlignment = other.verticalAlignment;
    wrapText = other.wrapText;

    return *this;
}
