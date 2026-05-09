#include "ssheetdrawing.h"

#include <QPainter>

SSheetDrawing::SSheetDrawing() :
    address(0, 0) {}

SSheetDrawing::SSheetDrawing(const SSheetDrawing &other) :
    address(other.address),
    offset(other.offset),
    size(other.size),
    image(other.image) {}

SSheetDrawing &SSheetDrawing::operator=(const SSheetDrawing &other)
{
    address = other.address;
    offset = other.offset;
    size = other.size;
    image = other.image;

    return *this;
}

void SSheetDrawing::render(QPainter *painter)
{
    painter->save();

    painter->translate(offset);
    painter->drawImage(0, 0, image.scaled(size.width(), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    painter->restore();
}
