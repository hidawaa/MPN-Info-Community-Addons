#ifndef STANDARDITEMDELEGATE_H
#define STANDARDITEMDELEGATE_H

#include <QStyledItemDelegate>

class StandardItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit StandardItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QString displayText(const QVariant &value, const QLocale &locale) const override;
};

#endif // STANDARDITEMDELEGATE_H
