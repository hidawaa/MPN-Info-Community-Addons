#ifndef KLUSELECTDIALOG_H
#define KLUSELECTDIALOG_H

#include <QDialog>

class QLineEdit;
class QTableView;
class CoreEngine;

class KluSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KluSelectDialog(CoreEngine *engine, QWidget *parent = nullptr);

    // Returns selected KLU code, empty if cancelled
    QString selectedKlu() const { return mSelectedKlu; }

private slots:
    void search();
    void accept() override;
    void onDoubleClicked();

private:
    CoreEngine *mEngine;
    QTableView *mResultView;
    QLineEdit *mKeywordEdit;
    QString mSelectedKlu;
};

#endif // KLUSELECTDIALOG_H
