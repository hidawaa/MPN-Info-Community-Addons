#include "loadingdialog.h"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

LoadingDialog::LoadingDialog(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint)
{
    mMessageLabel = new QLabel("Tunggu Sebentar...");
    mMessageLabel->setAlignment(Qt::AlignCenter);

    mProgressBar = new QProgressBar;
    mProgressBar->setTextVisible(false);
    mProgressBar->setRange(0, -1);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mMessageLabel, 1);
    layout->addWidget(mProgressBar);

    setLayout(layout);
    setWindowTitle("Loading");
    resize(400, 50);
}

void LoadingDialog::setMessage(const QString &message)
{
    mMessageLabel->setText(message);
}

void LoadingDialog::setProgress(int value, int range)
{
    mProgressBar->setRange(0, range);
    mProgressBar->setValue(value);
}
