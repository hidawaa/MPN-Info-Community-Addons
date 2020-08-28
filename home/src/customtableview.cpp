#include "customtableview.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QMimeData>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

#include "standarditemdelegate.h"

#include "../define.h"
#include "../spreadsheet.h"

class CustomSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    CustomSortFilterProxyModel(QObject *parent = 0) :
        QSortFilterProxyModel(parent) {}

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        if (!left.data(RoleGroup).isNull() && !right.data(RoleGroup).isNull())
        {
            QString leftGroup = left.data(RoleGroup).toString();
            int leftGroupLevel = left.data(RoleGroupLevel).toInt();

            QString rightGroup = right.data(RoleGroup).toString();
            int rightGroupLevel = right.data(RoleGroupLevel).toInt();

            int minLevel = qMin(leftGroupLevel, rightGroupLevel);
            int compareLevel(0);
            QString leftCompareGroup, rightCompareGroup;
            while (true)
            {
                leftCompareGroup = leftGroup.section(";", 0, compareLevel);
                rightCompareGroup = rightGroup.section(";", 0, compareLevel);

                if (leftCompareGroup != rightCompareGroup)
                    break;

                compareLevel++;

                if (compareLevel > minLevel)
                {
                    if (leftGroupLevel != rightGroupLevel)
                        return sortOrder() == Qt::AscendingOrder? leftGroupLevel < rightGroupLevel : leftGroupLevel > rightGroupLevel;

                    return left.data() < right.data();
                }
            }

            QModelIndex leftParentIndex = leftGroupLevel > compareLevel? searchGroup(leftCompareGroup, compareLevel, left.column()) : left;
            QModelIndex rightParentIndex = rightGroupLevel > compareLevel? searchGroup(rightCompareGroup, compareLevel, right.column()) : right;

            if (leftParentIndex.data() == rightParentIndex.data())
                return leftParentIndex.row() < rightParentIndex.row();

            return leftParentIndex.data() < rightParentIndex.data();
        }

        if (!left.data(RoleFooter).isNull() || !right.data(RoleFooter).isNull())
        {
            bool isLeftFooter = left.data(RoleFooter).toBool();
            bool isRightFooter = right.data(RoleFooter).toBool();

            if (isLeftFooter)
                return sortOrder() == Qt::AscendingOrder? false : true;

            if (isRightFooter)
                return sortOrder() == Qt::AscendingOrder? true : false;
        }

        return QSortFilterProxyModel::lessThan(left, right);
    }

    QModelIndex searchGroup(const QVariant &group, int level, int column) const
    {
        if (!sourceModel())
            return QModelIndex();

        for (int i=0; i<sourceModel()->rowCount(); i++) {
            QModelIndex tempIndex = sourceModel()->index(i, column);
            if (tempIndex.data(RoleGroupLevel).toInt() != level)
                continue;

            if (tempIndex.data(RoleGroup) == group)
                return tempIndex;
        }

        return QModelIndex();
    }
};

CustomTableView::CustomTableView(QWidget *parent) :
    QTableView(parent),
    mModel(0),
    mContextMenuEnabled(true)
{
    mProxyModel = new CustomSortFilterProxyModel(this);

    mCopyAction = new QAction(QIcon(IDR_COPY), "Copy", this);
    connect(mCopyAction, SIGNAL(triggered()), SLOT(copy()));

    mContextMenu = new QMenu(this);
    mContextMenu->addAction(mCopyAction);

    QFont defaultFont = font();
    defaultFont.setPointSize(8);
    setFont(defaultFont);

    setSortingEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWordWrap(false);
    setAlternatingRowColors(true);
    setItemDelegate(new StandardItemDelegate(this));

    horizontalHeader()->setHighlightSections(false);
    verticalHeader()->setHighlightSections(false);
    verticalHeader()->setDefaultSectionSize(fontMetrics().height()+10);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

    QTableView::setModel(mProxyModel);
}

void CustomTableView::setModel(QAbstractItemModel *model)
{
    mModel = model;
    mProxyModel->setSourceModel(model);
}

QAbstractItemModel *CustomTableView::model()
{
    return mModel;
}

QAbstractItemModel *CustomTableView::proxyModel()
{
    return mProxyModel;
}

void CustomTableView::setContextMenuEnabled(bool enable)
{
    mContextMenuEnabled = enable;
}

QMenu *CustomTableView::contextMenu()
{
    return mContextMenu;
}

void CustomTableView::setContextMenu(QMenu *menu)
{
    mContextMenu = menu;
}

QModelIndexList CustomTableView::selectedIndexes() const
{
    return QTableView::selectedIndexes();
}

void CustomTableView::copy()
{
    QModelIndexList indexList = selectedIndexes();
    int minRow(-1), minCol(-1);
    foreach (const QModelIndex &model, indexList) {
        if (minRow == -1 || model.row() < minRow)
            minRow = model.row();

        if (minCol == -1 || model.column() < minCol)
            minCol = model.column();
    }

    Spreadsheet s;
    s.createWorksheet("Copy");
    foreach (const QModelIndex &model, indexList)
        s.setValue(model.row() - minRow, model.column() - minCol, model.data());

    QClipboard *clipboard = QApplication::clipboard();

    QMimeData *mimeData = new QMimeData;
    mimeData->setHtml(s.toHtml());
    mimeData->setText(s.toPlainText());

    clipboard->setMimeData(mimeData);
}

void CustomTableView::customMenuRequested(QPoint pos)
{
    if (!mContextMenuEnabled)
        return;

    mContextMenu->popup(viewport()->mapToGlobal(pos));
}

void CustomTableView::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Copy))
        copy();
    else
        QTableView::keyPressEvent(event);
}
