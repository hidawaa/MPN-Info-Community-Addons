#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QChartGlobal>

#include <coreengine.h>

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
QT_CHARTS_END_NAMESPACE

class QLabel;
class QComboBox;
class QStackedWidget;
class QTabWidget;
class PenerimaanDialer;
class Controller;
class HomeFunction;
class CustomTableView;
class CustomChartView;

QT_CHARTS_USE_NAMESPACE

class HomePage : public Page
{
    Q_OBJECT
public:
    explicit HomePage(CoreEngine *engine, QWidget *parent = 0);

public slots:
    void refresh();

private slots:
    void gotData(const QVariantMap &data);
    void updateResult();
    void onWinnerButtonClicked();

private:
    CoreEngine *mEngine;
    HomeFunction *mFunction;

    QStackedWidget *mStackedWidget;

    QTabWidget *mTabWidget;
    CustomTableView *mResultView;

    QComboBox *mKantorComboBox;
    QComboBox *mYearComboBox;

    PenerimaanDialer *mDialer;
    CustomChartView *mBandingAkumulasiPenerimaanChart;
    CustomChartView *mRasioMpnSpmChart;
    QStackedWidget *mSeksiStackedWidget;

    QLabel *mLastMpnLabel;
    QLabel *mLastSpmLabel;
    QLabel *mVersionLabel;

    Kantor mKantor;
    QStringList mKantorList;

    QHash<int, QHash<int, double> > mTotalMpnBulanCurrentYearHash;
    QHash<int, QHash<int, double> > mTotalMpnBulanLastYearHash;
    QHash<int, QHash<int, double> > mTotalSpmBulanCurrentYearHash;
    QHash<int, QHash<int, double> > mTotalSpmBulanLastYearHash;
    QHash<int, QHash<int, double> > mTotalSpmkpCurrentYearHash;
    QHash<int, QHash<int, double> > mTotalSpmkpLastYearHash;
    QHash<int, QHash<int, double> > mTotalSpmppCurrentYearHash;
    QHash<int, QHash<int, double> > mTotalSpmppLastYearHash;
    QHash<int, QHash<int, double> > mTotalPbkCurrentYearHash;
    QHash<int, QHash<int, double> > mTotalPbkLastYearHash;
    QHash<int, QHash<int, double> > mTotalRenpenCurrentYearHash;

    QHash<int, PenerimaanDialer *> mDialerSeksiHash;
    QHash<int, CustomChartView *> mBandingAkumulasiSeksiChartHash;
    QHash<int, CustomChartView *> mBandingBulanSeksiChartHash;
};

#endif // HOMEPAGE_H
