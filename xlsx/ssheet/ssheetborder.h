#ifndef SSHEETBORDER_H
#define SSHEETBORDER_H


class SSheetBorder
{
public:
    enum Position {
        Unknown = 0,
        Top,
        Bottom,
        Left,
        Right
    };

    enum Type {
        None = 0,
        Thin,
        Medium,
        Thick,
        Dot,
        Dash,
        DashDot,
        DashDotDot
    };

    SSheetBorder();
    SSheetBorder(const SSheetBorder &other);
    SSheetBorder &operator=(const SSheetBorder &other);

    Type top;
    Type bottom;
    Type left;
    Type right;
};

#endif // SSHEETBORDER_H
