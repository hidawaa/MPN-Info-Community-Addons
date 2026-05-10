#ifndef MASTERFILESEARCHPAGE_H
#define MASTERFILESEARCHPAGE_H

#include <QWidget>
#include <QVariantMap>

class QLineEdit;
class QDateEdit;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QMenu;
class QAction;

class CoreEngine;
class QTableView;
class MasterfileSearchFunction;

#include <interface.h>

class MasterfileSearchPage : public Page
{
    Q_OBJECT
public:
    explicit MasterfileSearchPage(CoreEngine *engine, QWidget *parent = nullptr);
    ~MasterfileSearchPage();

    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
    void clear();
    void search();
    void save();

private slots:
    void appendRows(const QList<QVariantList> &rows);
    void searchFinished(int count);
    void updateFilter();
    void updateFilterKantor();
    void selectKlu();
    void copySelection();
    void showContextMenu(const QPoint &pos);

private:
    CoreEngine *mEngine;
    MasterfileSearchFunction *mFunction;

    QTableView *mResultView;
    QLabel *mStatusLabel;
    QComboBox *mKantorComboBox;
    QCheckBox *mNpwpCheckBox;
    QLineEdit *mNpwpLineEdit;
    QCheckBox *mNamaCheckBox;
    QLineEdit *mNamaLineEdit;
    QCheckBox *mKluCheckBox;
    QLineEdit *mKluLineEdit;
    QPushButton *mKluSelectButton;
    QCheckBox *mAlamatCheckBox;
    QLineEdit *mAlamatLineEdit;
    QCheckBox *mJenisWpCheckBox;
    QComboBox *mJenisWpComboBox;
    QCheckBox *mStatusWpCheckBox;
    QComboBox *mStatusWpComboBox;
    QCheckBox *mTglDaftarCheckBox;
    QDateEdit *mTglDaftarStartDateEdit;
    QDateEdit *mTglDaftarEndDateEdit;
    QCheckBox *mSeksiCheckBox;
    QComboBox *mSeksiComboBox;
    QCheckBox *mArCheckBox;
    QComboBox *mPjComboBox;

    QMenu *mContextMenu;
    QAction *mCopyAction;
};

#endif // MASTERFILESEARCHPAGE_H
