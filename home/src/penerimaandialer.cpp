#include "penerimaandialer.h"

#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>

#include "ui/dialer.h"

PenerimaanDialer::PenerimaanDialer(QWidget *parent) :
    QWidget(parent),
    mPenerimaan(0),
    mSpmkp(0),
    mSpmpp(0),
    mRenpen(0),
    mNettoTahunLalu(0),
    mCapaian(0),
    mTumbuh(0)

{
    mDialer = new Dialer;
    mTitleLabel = new QLabel;
    mTotalPenerimaanLabel =  new QLabel;
    mTotalSpmkpLabel =  new QLabel;
    mTotalPbkLabel =  new QLabel;
    mTotalNettoLabel =  new QLabel;
    mTotalRenpenLabel =  new QLabel;
    mPencapaianLabel =  new QLabel;
    mPertumbuhanLabel =  new QLabel;

    QFont titleFont = mTitleLabel->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(14);
    mTitleLabel->setFont(titleFont);
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mTitleLabel->setMinimumWidth(250);

    QGridLayout *penerimaanLayout = new QGridLayout;
    penerimaanLayout->setMargin(0);
    penerimaanLayout->addWidget(new QLabel("Total Penerimaan"), 0, 0);
    penerimaanLayout->addWidget(mTotalPenerimaanLabel, 0, 1, Qt::AlignRight);
    penerimaanLayout->addWidget(new QLabel("Total SPMKP dan SPMPP"), 1, 0);
    penerimaanLayout->addWidget(mTotalSpmkpLabel, 1, 1, Qt::AlignRight);
    penerimaanLayout->addWidget(new QLabel("Total PBK"), 2, 0);
    penerimaanLayout->addWidget(mTotalPbkLabel, 2, 1, Qt::AlignRight);
    penerimaanLayout->addWidget(new QLabel("Total Netto"), 3, 0);
    penerimaanLayout->addWidget(mTotalNettoLabel, 3, 1, Qt::AlignRight);
    penerimaanLayout->addWidget(new QLabel("Total Renpen"), 4, 0);
    penerimaanLayout->addWidget(mTotalRenpenLabel, 4, 1, Qt::AlignRight);
    penerimaanLayout->addWidget(new QLabel("Pencapaian"), 5, 0);
    penerimaanLayout->addWidget(mPencapaianLabel, 5, 1, Qt::AlignRight);
    penerimaanLayout->addWidget(new QLabel("Pertumbuhan"), 6, 0);
    penerimaanLayout->addWidget(mPertumbuhanLabel, 6, 1, Qt::AlignRight);

    QWidget *textWidget = new QWidget;
    textWidget->setLayout(penerimaanLayout);
    textWidget->setMinimumWidth(250);
    textWidget->setMaximumWidth(250);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mTitleLabel, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(mDialer, 0, Qt::AlignCenter);
    layout->addWidget(textWidget, 0, Qt::AlignCenter);

    setLayout(layout);
    setMinimumWidth(270);
    setMaximumWidth(270);

    replot();
}

void PenerimaanDialer::setTitle(const QString &title)
{
    mTitleLabel->setText(title);
}

void PenerimaanDialer::setPenerimaan(double value)
{
    mPenerimaan = value;
    replot();
}

void PenerimaanDialer::setSpmkp(double value)
{
    mSpmkp = value;
    replot();
}

void PenerimaanDialer::setSpmpp(double value)
{
    mSpmpp = value;
    replot();
}

void PenerimaanDialer::setPbk(double value)
{
    mPbk = value;
    replot();
}

void PenerimaanDialer::setRenpen(double value)
{
    mRenpen = value;
    replot();
}

void PenerimaanDialer::setNettoTahunlalu(double value)
{
    mNettoTahunLalu = value;
    replot();
}

void PenerimaanDialer::replot()
{
    double pengurang = mSpmkp + mSpmpp;
    double netto = mPenerimaan - pengurang + mPbk;
    mCapaian = 0;
    if (mRenpen)
        mCapaian = netto * 100.0 / mRenpen;

    mTumbuh = 0;
    if (mNettoTahunLalu)
        mTumbuh = (netto - mNettoTahunLalu) * 100.0 / mNettoTahunLalu;

    mDialer->setValue(mCapaian);
    mTotalPenerimaanLabel->setText(QString("%L1").arg(mPenerimaan, 0, 'f', 2));
    mTotalSpmkpLabel->setText(QString("%L1").arg(pengurang, 0, 'f', 2));
    mTotalPbkLabel->setText(QString("%L1").arg(mPbk, 0, 'f', 2));
    mTotalNettoLabel->setText(QString("%L1").arg(netto, 0, 'f', 2));
    mTotalRenpenLabel->setText(QString("%L1").arg(mRenpen, 0, 'f', 2));
    mPencapaianLabel->setText(QString("%1%").arg(mCapaian, 0, 'f', 2));
    mPertumbuhanLabel->setText(QString("%1%").arg(mTumbuh, 0, 'f', 2));
}

float PenerimaanDialer::capaian()
{
    return mCapaian;
}

float PenerimaanDialer::pertumbuhan()
{
    return mTumbuh;
}

