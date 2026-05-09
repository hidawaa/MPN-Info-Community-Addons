#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include <QTableView>

class CustomSortFilterProxyModel;
class CustomTableView : public QTableView
{
    Q_OBJECT
public:
    explicit CustomTableView(QWidget *parent = 0);

    QAbstractItemModel *proxyModel();
    QAbstractItemModel *model();
    void setModel(QAbstractItemModel *model);

    void setContextMenuEnabled(bool enable);
    QMenu *contextMenu();
    void setContextMenu(QMenu *menu);

    QModelIndexList selectedIndexes() const;

public slots:
    void copy();

private slots:
    void customMenuRequested(QPoint pos);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QAbstractItemModel *mModel;
    CustomSortFilterProxyModel *mProxyModel;

    bool mContextMenuEnabled;
    QMenu *mContextMenu;
    QAction *mCopyAction;
};

#endif // CUSTOMTABLEVIEW_H
