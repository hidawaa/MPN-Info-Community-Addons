#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>

class QLabel;
class QProgressBar;
class LoadingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadingDialog(QWidget *parent = nullptr);

public slots:
    void setMessage(const QString &message);
    void setProgress(int value, int range);

private:
    QLabel *mMessageLabel;
    QProgressBar *mProgressBar;
};

#endif // LOADINGDIALOG_H
