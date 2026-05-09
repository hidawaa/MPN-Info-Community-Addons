#ifndef SSHEETDRAWING_H
#define SSHEETDRAWING_H

#include <QPointF>
#include <QSizeF>
#include <QImage>

#include "ssheetaddress.h"

class QPainter;
class SSheetDrawing
{
public:
    SSheetDrawing();
    SSheetDrawing(const SSheetDrawing &other);
    SSheetDrawing &operator=(const SSheetDrawing &other);

    void render(QPainter *painter);

    SSheetAddress address;
    QPointF offset;
    QSize size;
    QImage image;
};

#endif // SSHEETDRAWING_H
