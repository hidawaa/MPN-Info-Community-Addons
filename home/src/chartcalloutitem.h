#ifndef CHARTCALLOUTITEM_H
#define CHARTCALLOUTITEM_H

#include <QFont>
#include <QGraphicsItem>

class ChartCalloutItem : public QGraphicsItem
{
public:
    ChartCalloutItem(QGraphicsItem *parent=0);

    QRectF boundingRect() const;
    void setText(const QString &text);
    QString text() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString mText;
    QFont mFont;
};

#endif // CHARTCALLOUTITEM_H
