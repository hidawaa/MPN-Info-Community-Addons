#ifndef DIALER_H
#define DIALER_H

#include <QWidget>

class Dialer : public QWidget
{
    Q_OBJECT
public:
    explicit Dialer(QWidget *parent = 0);

    int value() { return mValue; }
    int minimum() { return mMinimum; }
    int maximum() { return mMaximum; }

    void setValue(int value);
    void setRange(int min, int max);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    int mMinimum;
    int mMaximum;
    int mValue;
};

#endif // DIALER_H
