#ifndef SSHEETSTYLE_H
#define SSHEETSTYLE_H

#include <QFont>
#include "ssheetborder.h"

class SSheetStyle
{
public:
    SSheetStyle();
    SSheetStyle(const SSheetStyle &other);
    SSheetStyle &operator=(const SSheetStyle &other);

    QFont font;
    SSheetBorder border;
    int horizontalAlignment;
    int verticalAlignment;
    bool wrapText;
};

#endif // SSHEETSTYLE_H
