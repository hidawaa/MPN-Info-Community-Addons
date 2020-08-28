#include "dialer.h"

#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>

Dialer::Dialer(QWidget *parent) :
    QWidget(parent),
    mMinimum(0),
    mMaximum(120),
    mValue(0)
{
    setMinimumWidth(210);
    setMinimumHeight(210);
    setMaximumWidth(210);
    setMaximumHeight(210);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void Dialer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.drawImage(QRect(0, 0, width(), height()), QImage(":/images/dial-background.png"));

    int angle = qMin(qMax(-130, int((mMinimum + mValue) * 260.0 / (mMaximum - mMinimum)  - 130)), 133);

    QRect needleRect;
    needleRect.setX(0);
    needleRect.setY(0);
    needleRect.setWidth(width() * 0.0380952380952381);
    needleRect.setHeight(height() * 0.3);

    QTransform trans;
    trans.translate(width() * 0.48, size().height() * 0.48);
    trans.rotate(angle);
    trans.translate(-needleRect.width() * 0.5, -needleRect.height() * 0.9);

    p.save();
    p.setTransform(trans);
    p.drawImage(needleRect, QImage(":/images/dial-needle.png"));
    p.restore();

    QRect overlayRect;
    overlayRect.setX(width() * 0.1);
    overlayRect.setY(height() * 0.0857142857142857);
    overlayRect.setWidth(width() * 0.7047619047619048);
    overlayRect.setHeight(height() * 0.5);
    p.drawImage(overlayRect, QImage(":/images/dial-overlay.png"));

    p.end();
}

void Dialer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    update();
}

void Dialer::setValue(int value)
{
    mValue = value;
    update();
}

void Dialer::setRange(int min, int max)
{
    mMinimum = min;
    mMaximum = max;
    update();
}
