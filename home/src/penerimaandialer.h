#ifndef PENERIMAANDIALER_H
#define PENERIMAANDIALER_H

#include <QWidget>

class Dialer;
class QLabel;
class PenerimaanDialer : public QWidget
{
    Q_OBJECT
public:
    explicit PenerimaanDialer(QWidget *parent = 0);

    void setTitle(const QString &title);
    void setPenerimaan(double value);
    void setSpmkp(double value);
    void setSpmpp(double value);
    void setPbk(double value);
    void setRenpen(double value);
    void setNettoTahunlalu(double value);

    float capaian();
    float pertumbuhan();

private slots:
    void replot();

private:
    Dialer *mDialer;
    QLabel *mTitleLabel;
    QLabel *mTotalPenerimaanLabel;
    QLabel *mTotalSpmkpLabel;
    QLabel *mTotalPbkLabel;
    QLabel *mTotalNettoLabel;
    QLabel *mTotalRenpenLabel;
    QLabel *mPencapaianLabel;
    QLabel *mPertumbuhanLabel;

    double mPenerimaan;
    double mSpmkp;
    double mSpmpp;
    double mPbk;
    double mRenpen;
    double mNettoTahunLalu;
    float mCapaian;
    float mTumbuh;
};

#endif // PENERIMAANDIALER_H
