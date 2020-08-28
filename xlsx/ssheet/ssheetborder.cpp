#include "ssheetborder.h"

SSheetBorder::SSheetBorder() :
    top(None),
    bottom(None),
    left(None),
    right(None) {}

SSheetBorder::SSheetBorder(const SSheetBorder &other) :
    top(other.top),
    bottom(other.bottom),
    left(other.left),
    right(other.right) {}

SSheetBorder &SSheetBorder::operator=(const SSheetBorder &other)
{
    top = other.top;
    bottom = other.bottom;
    left = other.left;
    right = other.right;

    return *this;
}
