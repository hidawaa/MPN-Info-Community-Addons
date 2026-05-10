#include "chartcalloutitem.h"

#include <QFontMetrics>
#include <QPainter>

ChartCalloutItem::ChartCalloutItem(QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    mFont.setBold(true);
}

QRectF ChartCalloutItem::boundingRect() const
{
    QFontMetrics metrics(mFont);
    QRectF rect = metrics.boundingRect(QRect(), Qt::AlignCenter, mText);
    return QRectF(-(rect.width() / 2 + 5), -(rect.height() + 11), rect.width() + 10, rect.height() + 11);
}

void ChartCalloutItem::setText(const QString &text)
{
    prepareGeometryChange();
    mText = text;
}

QString ChartCalloutItem::text() const
{
    return mText;
}

void ChartCalloutItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QBrush brush = painter->brush();
    brush.setColor(QColor(128, 128, 128, 192));
    painter->setBrush(brush);

    QPen pen = painter->pen();
    pen.setColor(Qt::black);
    painter->setPen(pen);

    painter->setFont(mFont);

    QRectF bRect = boundingRect();
    QRectF fRect(0, 0, bRect.width(), bRect.height() - 5);
    painter->translate(bRect.x(), bRect.y());
    painter->drawRect(fRect);
    painter->fillRect(fRect, QColor(245, 245, 245, 225));
    painter->drawText(fRect, Qt::AlignCenter, mText);
}
