#include "standarditemdelegate.h"

#include <QPainter>
#include <QVariant>
#include <QDate>
#include <QLocale>

StandardItemDelegate::StandardItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void StandardItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    // Paint negative numbers in dark red
    switch (index.data().typeId()) {
    case QMetaType::Double:
    case QMetaType::Float:
    {
        if (index.data().toDouble() < 0) {
            QStyleOptionViewItem viewOption(option);
            viewOption.palette.setColor(QPalette::Text, Qt::darkRed);
            viewOption.palette.setColor(QPalette::HighlightedText, Qt::darkRed);
            QStyledItemDelegate::paint(painter, viewOption, index);
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
        break;
    }
    default:
        QStyledItemDelegate::paint(painter, option, index);
        break;
    }
}

QString StandardItemDelegate::displayText(const QVariant &value,
                                          const QLocale & /*locale*/) const
{
    // Use Indonesian locale for number formatting (period = thousands sep, comma = decimal)
    QLocale idLocale(QLocale::Indonesian, QLocale::Indonesia);

    switch (value.typeId()) {
    case QMetaType::QDate:
        return value.toDate().toString("dd-MM-yyyy");

    case QMetaType::Double:
        // Format with thousands separator, 2 decimal places
        return idLocale.toString(value.toDouble(), 'f', 2);

    case QMetaType::UInt:
        // Format as integer with thousands separator
        return idLocale.toString(value.toDouble(), 'f', 0);

    case QMetaType::Float:
        // Format as percentage with 2 decimal places
        return idLocale.toString(value.toFloat() * 100.0, 'f', 2) + QChar('%');

    default:
        return value.toString();
    }
}
