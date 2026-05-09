#include "homepage.h"
#include "homepage_p.h"

#include <qmath.h>
#include <QDate>
#include <QComboBox>
#include <QTimer>
#include <QGridLayout>
#include <QSqlQuery>
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QScrollArea>
#include <QPalette>
#include <QPushButton>

#include "customchartview.h"
#include <QStackedBarSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QLineSeries>
#include <QLegend>
#include <QChart>
#include <QCategoryAxis>
#include <QBarCategoryAxis>
#include <QValueAxis>

#include "customtableview.h"
#include "dialer.h"
#include "penerimaandialer.h"

QT_CHARTS_USE_NAMESPACE

HomePage::HomePage(CoreEngine *engine, QWidget *parent) :
    Page(parent),
    mEngine(engine)
{
    mFunction = new HomeFunction(mEngine, this);

    mKantorComboBox = new QComboBox;
    QString kodeKantor = mEngine->databaseSettings()->value("kantor.kode").toString();
    Kantor kantor = mEngine->kantor(kodeKantor);

    if (kantor.kpp.isEmpty()) {
        foreach (const Kantor &tempKantor, mEngine->kppList(kantor.kanwil))
            mKantorComboBox->addItem(tempKantor.nama, tempKantor.kpp);
    }
    else
        mKantorComboBox->addItem(kantor.nama, kantor.kpp);

    QDate currentDate = QDate::currentDate();
    mYearComboBox = new QComboBox;

    mYearComboBox->addItem(QString::number(currentDate.year()), currentDate.year());
    QString tahunSql = QString("SELECT DISTINCT `tahunbayar` FROM `mpn` ORDER BY `tahunbayar` DESC");
    QSqlQuery tahunQuery(tahunSql, mEngine->database());
    while (tahunQuery.next()) {
        int tahun = tahunQuery.value(0).toInt();
        if (tahun == currentDate.year())
            continue;

        mYearComboBox->addItem(QString::number(tahun), tahun);
    }
    mYearComboBox->setCurrentIndex(mYearComboBox->findData(currentDate.year()));

    QHBoxLayout *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel("Kantor"));
    filterLayout->addWidget(mKantorComboBox);
    filterLayout->addWidget(new QLabel("Tahun Pembayaran"));
    filterLayout->addWidget(mYearComboBox);
    filterLayout->addStretch();

    mTabWidget = new QTabWidget;
    mTabWidget->setTabPosition(QTabWidget::West);

    QFont legendFont;
    legendFont.setFamily("Arial");
    legendFont.setPixelSize(10);

    // Dialer
    mDialer = new PenerimaanDialer;

    // Total Penerimaan Bulan
    mBandingAkumulasiPenerimaanChart = new CustomChartView;
    mBandingAkumulasiPenerimaanChart->setBackgroundBrush(QBrush(Qt::white));

    {
        QLineSeries *renpenSeries = new QLineSeries;
        QLineSeries *penerimaanSeries = new QLineSeries;
        QLineSeries *penerimaanLastSeries = new QLineSeries;

        QCategoryAxis *axisX = new QCategoryAxis;
        for (int bulan=1; bulan <= 12; bulan++)
            axisX->append(Common::toNamaBulan(bulan), bulan);
        axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
        axisX->setMin(0);
        axisX->setMax(13);

        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText("Rupiah (Miliar)");

        QChart *chart = new QChart;
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setMargins(QMargins(0, 0, 0, 0));

        QFont font = chart->titleFont();
        font.setPixelSize(14);
        font.setBold(true);
        chart->setTitle("Perbandingan Akumulasi Penerimaan");
        chart->setTitleFont(font);

        chart->addSeries(renpenSeries);
        chart->addSeries(penerimaanSeries);
        chart->addSeries(penerimaanLastSeries);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        foreach (QAbstractSeries *series, chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }

        chart->legend()->setFont(legendFont);
        chart->legend()->detachFromChart();
        chart->legend()->setAlignment(Qt::AlignLeft);
        chart->legend()->setMinimumSize(118, 80);
        chart->legend()->setBackgroundVisible(true);
        chart->legend()->setBrush(QBrush(QColor(128, 128, 128, 32)));
        chart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));

        mBandingAkumulasiPenerimaanChart->setChart(chart);
        mBandingAkumulasiPenerimaanChart->setRenderHint(QPainter::Antialiasing);
    }

    // Rasio MPN dan SPM
    mRasioMpnSpmChart = new CustomChartView;
    mRasioMpnSpmChart->setBackgroundBrush(QBrush(Qt::white));

    {
        QBarSeries *barSeries = new QBarSeries;

        QBarCategoryAxis *axisX = new QBarCategoryAxis;
        for (int bulan=1; bulan <= 12; bulan++)
            axisX->append(Common::toNamaBulan(bulan));

        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText("Rupiah (Miliar)");

        QChart *chart = new QChart;
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setMargins(QMargins(0, 0, 0, 0));

        QFont font = chart->titleFont();
        font.setPixelSize(14);
        font.setBold(true);
        chart->setTitle("Perbandingan Penerimaan Per Bulan");
        chart->setTitleFont(font);

        chart->addSeries(barSeries);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        foreach (QAbstractSeries *series, chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }

        chart->legend()->setFont(legendFont);
        chart->legend()->detachFromChart();
        chart->legend()->setAlignment(Qt::AlignLeft);
        chart->legend()->setMinimumSize(258, 60);
        chart->legend()->setBackgroundVisible(true);
        chart->legend()->setBrush(QBrush(QColor(128, 128, 128, 32)));
        chart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));

        mRasioMpnSpmChart->setChart(chart);
        mRasioMpnSpmChart->setRenderHint(QPainter::Antialiasing);
    }

    QFont versionFont;
    versionFont.setBold(true);
    versionFont.setPixelSize(11);

    mLastMpnLabel = new QLabel;
    mLastSpmLabel = new QLabel;
    mVersionLabel = new QLabel;

    mLastMpnLabel->setFont(versionFont);
    mLastSpmLabel->setFont(versionFont);
    mVersionLabel->setFont(versionFont);

    QWidget *leftSideWidget = new QWidget;
    leftSideWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    QVBoxLayout *leftSideLayout = new QVBoxLayout;
    leftSideLayout->setMargin(0);
    leftSideWidget->setLayout(leftSideLayout);
    leftSideLayout->addWidget(mDialer, 1, Qt::AlignCenter);
    leftSideLayout->addWidget(mLastMpnLabel);
    leftSideLayout->addWidget(mLastSpmLabel);
    leftSideLayout->addWidget(mVersionLabel);

    QGridLayout *kantorLayout = new QGridLayout;
    kantorLayout->addWidget(leftSideWidget, 1, 0, 2, 1);
    kantorLayout->addWidget(mBandingAkumulasiPenerimaanChart, 1, 1);
    kantorLayout->addWidget(mRasioMpnSpmChart, 2, 1);

    QWidget *kantorWidget = new QWidget;
    kantorWidget->setLayout(kantorLayout);
    kantorWidget->setAutoFillBackground(true);
    kantorWidget->setBackgroundRole(QPalette::Light);

    mSeksiStackedWidget = new QStackedWidget;

    mResultView = new CustomTableView;
    mResultView->verticalHeader()->hide();
    mResultView->horizontalHeader()->hide();
    mResultView->setStyleSheet("QTableView { border: none; }");
    mResultView->setAlternatingRowColors(false);
    mResultView->setGridStyle(Qt::DotLine);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setMargin(0);
    textLayout->addWidget(mResultView);

    QWidget *textWidget = new QWidget;
    textWidget->setLayout(textLayout);

    mTabWidget->addTab(kantorWidget, "Kantor");
    mTabWidget->addTab(mSeksiStackedWidget, "Seksi");
    mTabWidget->addTab(textWidget, "Angka");

    QHBoxLayout *loadingLayout = new QHBoxLayout;
    loadingLayout->addWidget(new QLabel("Memuat Data..."), 1, Qt::AlignCenter);

    QWidget *loadingWidget = new QWidget;
    loadingWidget->setLayout(loadingLayout);

    QVBoxLayout *winnerLayout = new QVBoxLayout;
    QWidget *winnerWidget = new QWidget;
    winnerWidget->setLayout(winnerLayout);

    {
        QPushButton *winnerOkButton = new QPushButton("OK! Yeah!!!");
        connect(winnerOkButton, SIGNAL(clicked()), SLOT(onWinnerButtonClicked()));

        QLabel *winnerImageLabel = new QLabel;
        winnerImageLabel->setAlignment(Qt::AlignCenter);
        winnerImageLabel->setPixmap(QPixmap(":/images/winner.png"));

        QLabel *winnerLabel = new QLabel;
        winnerLabel->setAlignment(Qt::AlignCenter);

        QFont winnerFont = winnerLabel->font();
        winnerFont.setPixelSize(14);

        winnerLabel->setFont(winnerFont);
        winnerLabel->setTextFormat(Qt::RichText);
        winnerLabel->setText(QString("Selamat kepada seluruh pegawai <b>%1</b>, karena capaian realisasi penerimaan kantor sudah <b><i>100%</i></b> !!!").arg(kantor.nama()));

        QHBoxLayout *winnerButtonLayout = new QHBoxLayout;
        winnerButtonLayout->addStretch();
        winnerButtonLayout->addWidget(winnerOkButton);
        winnerButtonLayout->addStretch();

        winnerLayout->setSpacing(10);
        winnerLayout->addStretch();
        winnerLayout->addWidget(winnerImageLabel);
        winnerLayout->addWidget(winnerLabel);
        winnerLayout->addLayout(winnerButtonLayout);
        winnerLayout->addStretch();
    }

    mStackedWidget = new QStackedWidget;
    mStackedWidget->addWidget(loadingWidget);
    mStackedWidget->addWidget(mTabWidget);
    mStackedWidget->addWidget(winnerWidget);
    mStackedWidget->setCurrentIndex(0);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(filterLayout);
    layout->addWidget(mStackedWidget);

    setLayout(layout);
    setWindowIcon(QIcon(IDR_LOGO));

    connect(mFunction, SIGNAL(gotData(QVariantMap)), SLOT(gotData(QVariantMap)));
    connect(mFunction, SIGNAL(finished()), SLOT(updateResult()));
    connect(mKantorComboBox, SIGNAL(currentIndexChanged(int)), SLOT(refresh()));
    connect(mYearComboBox, SIGNAL(currentIndexChanged(int)), SLOT(refresh()));

    refresh();
}

void HomePage::refresh()
{
    delete mResultView->model();
    mPegawaiHash.clear();

    mDialerSeksiHash.clear();
    mBandingAkumulasiSeksiChartHash.clear();
    mBandingBulanSeksiChartHash.clear();

    QWidget *seksiWidget = mSeksiStackedWidget->widget(0);
    mSeksiStackedWidget->removeWidget(seksiWidget);
    delete seksiWidget;

    QString kodeKantor = mKantorComboBox->currentData().toString();
    mKantor = Kantor::getKantor(kodeKantor);
    if (mKantor.isKanwil())
    {
        QStringList tempList;
        foreach (const Kantor &kantor, Kantor::getKppKanwilList(kodeKantor))
            tempList << kantor.kpp();
        mKantorList = tempList;
    }
    else
        mKantorList = QStringList() << kodeKantor;

    mPegawaiHash = Pegawai::getPegawaiHash(kodeKantor, QDate::currentDate().year());

    mFunction->setKantor(mKantorComboBox->currentData().toString());
    mFunction->setTahun(mYearComboBox->currentData().toInt());

    mKantorComboBox->setEnabled(false);
    mYearComboBox->setEnabled(false);

    mStackedWidget->setCurrentIndex(0);
    mFunction->start();
}

void HomePage::gotData(const QVariantMap &data)
{
    int type = data["type"].toInt();
    QString npwp = data["npwp"].toString();
    QString kpp = data["kpp"].toString();
    QString cabang = data["cabang"].toString();
    QString npwp2 = data["npwp2"].toString();
    QString kpp2 = data["kpp2"].toString();
    QString cabang2 = data["cabang2"].toString();
    QString nip = data["nip"].toString();
    int bulan = data["bulan"].toInt();
    double nominal = data["nominal"].toDouble();

    int seksi(0), seksi2(0);
    switch (type)
    {
    case HomeFunction::DataMpnCurrentYear:
    case HomeFunction::DataMpnLastYear:
    case HomeFunction::DataSpmCurrentYear:
    case HomeFunction::DataSpmLastYear:
    case HomeFunction::DataSpmkpCurrentYear:
    case HomeFunction::DataSpmkpLastYear:
    case HomeFunction::DataSpmppCurrentYear:
    case HomeFunction::DataSpmppLastYear:
        seksi = mPegawaiHash[mEngine->wajibPajakHash()[npwp + kpp + cabang].nipPj()].seksi();
        break;
    case HomeFunction::DataPbkLastYear:
    case HomeFunction::DataPbkCurrentYear:
        seksi = mPegawaiHash[mEngine->wajibPajakHash()[npwp + kpp + cabang].nipPj()].seksi();
        seksi2 = mPegawaiHash[mEngine->wajibPajakHash()[npwp2 + kpp2 + cabang2].nipPj()].seksi();
        break;
    case HomeFunction::DataRenpenCurrentYear:
        seksi = mPegawaiHash[nip].seksi();
        break;
    default:
        break;
    }

    switch (type)
    {
    case HomeFunction::DataMpnCurrentYear:
        mTotalMpnBulanCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataMpnLastYear:
        mTotalMpnBulanLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataSpmCurrentYear:
        mTotalSpmBulanCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataSpmLastYear:
        mTotalSpmBulanLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataSpmkpCurrentYear:
        mTotalSpmkpCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataSpmkpLastYear:
        mTotalSpmkpLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataSpmppCurrentYear:
        mTotalSpmppCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataSpmppLastYear:
        mTotalSpmppLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeFunction::DataPbkLastYear:
    {
        if (mEngine->serverSetting("hitung.pbk").toBool()) {
            if (mKantorList.contains(kpp))
                mTotalPbkLastYearHash[seksi][bulan] -= nominal;

            if (mKantorList.contains(kpp2))
                mTotalPbkLastYearHash[seksi2][bulan] += nominal;
        }
        else {
            if (mKantorList.contains(mEngine->wajibPajakHash()[npwp + kpp + cabang].admin()))
                mTotalPbkLastYearHash[seksi][bulan] -= nominal;

            if (mKantorList.contains(mEngine->wajibPajakHash()[npwp2 + kpp2 + cabang2].admin()))
                mTotalPbkLastYearHash[seksi2][bulan] += nominal;
        }
    }
        break;
    case HomeFunction::DataPbkCurrentYear:
    {
        if (mEngine->serverSetting("hitung.pbk").toBool()) {
            if (mKantorList.contains(kpp))
                mTotalPbkCurrentYearHash[seksi][bulan] -= nominal;

            if (mKantorList.contains(kpp2))
                mTotalPbkCurrentYearHash[seksi2][bulan] += nominal;
        }
        else {
            if (mKantorList.contains(mEngine->wajibPajakHash()[npwp + kpp + cabang].admin()))
                mTotalPbkCurrentYearHash[seksi][bulan] -= nominal;

            if (mKantorList.contains(mEngine->wajibPajakHash()[npwp2 + kpp2 + cabang2].admin()))
                mTotalPbkCurrentYearHash[seksi2][bulan] += nominal;
        }
    }
        break;
    case HomeFunction::DataRenpenCurrentYear:
        mTotalRenpenCurrentYearHash[seksi][bulan] += nominal;
        break;
    default:
        break;
    }
}

void HomePage::updateResult()
{
    int year = mYearComboBox->currentData().toInt();
    float markCapai(0);
    double markRenpen(0);
    QDate currentDate = QDate::currentDate();
    QStandardItemModel *model = new QStandardItemModel(500, 50, mResultView);
    int angkaRow(0);
    QList<QHash<QString, int> > angkaHeaderList;

    QList<int> seksiIdList;
    seksiIdList << 0;

    if (mKantor.isKanwil())
    {
        mTabWidget->setTabEnabled(1, false);
    }
    else
    {
        mTabWidget->setTabEnabled(1, true);

        QPen gridPen, subGridPen;
        gridPen.setStyle(Qt::SolidLine);
        gridPen.setColor(QColor(0, 0, 0, 25));
        subGridPen.setColor(QColor(0, 0, 0, 25));
        subGridPen.setStyle(Qt::DotLine);

        QVector<double> bulanTicks;
        QVector<QString> bulanLabels;
        for (int bulan=1; bulan <= 12; bulan++) {
            bulanTicks << bulan;
            bulanLabels << Common::toNamaBulan(bulan);
        }

        int seksiPageHeight(0);
        QVBoxLayout *seksiLayout = new QVBoxLayout;
        seksiLayout->setMargin(0);

        QList<int> tipeSeksiList;
        if (year >= 2015)
            tipeSeksiList << SeksiPengawasanDanKonsultasiPengawasan << SeksiEkstensifikasiPerpajakan;
        else
            tipeSeksiList << SeksiPengawasanDanKonsultasiPelayanan << SeksiPengawasanDanKonsultasiPengawasan;

        QList<Seksi> seksiList = Seksi::getSeksiList(mKantor.kode(), tipeSeksiList);
        seksiPageHeight += 400 * seksiList.count();

        foreach (const Seksi &seksi, seksiList)
        {
            seksiIdList << seksi.id();

            PenerimaanDialer *dialer = new PenerimaanDialer;
            dialer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            dialer->setTitle(QString("Penerimaan Seksi\n%1").arg(seksi.nama()));

            mDialerSeksiHash[seksi.id()] = dialer;

            CustomChartView *chartView1 = new CustomChartView;
            chartView1->setBackgroundBrush(QBrush(Qt::white));

            {
                QLineSeries *renpenSeries = new QLineSeries;
                QLineSeries *penerimaanSeries = new QLineSeries;
                QLineSeries *penerimaanLastSeries = new QLineSeries;

                renpenSeries->setName(QString("Renpen %1").arg(year));
                penerimaanSeries->setName(QString("Realisasi %1").arg(year));
                penerimaanLastSeries->setName(QString("Realisasi %1").arg(year - 1));

                QCategoryAxis *axisX = new QCategoryAxis;
                for (int bulan=1; bulan <= 12; bulan++)
                    axisX->append(Common::toNamaBulan(bulan).left(3), bulan);
                axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
                axisX->setMin(0);
                axisX->setMax(13);

                QValueAxis *axisY = new QValueAxis;
                axisY->setTitleText("Rupiah (Milyar)");

                QChart *chart = new QChart;
                chart->setAnimationOptions(QChart::SeriesAnimations);
                chart->setMargins(QMargins(0, 0, 0, 0));

                QFont font = chart->titleFont();
                font.setPixelSize(14);
                font.setBold(true);
                chart->setTitle("Perbandingan Akumulasi Penerimaan");
                chart->setTitleFont(font);

                chart->addSeries(renpenSeries);
                chart->addSeries(penerimaanSeries);
                chart->addSeries(penerimaanLastSeries);

                chart->addAxis(axisX, Qt::AlignBottom);
                chart->addAxis(axisY, Qt::AlignLeft);

                foreach (QAbstractSeries *series, chart->series()) {
                    series->attachAxis(axisX);
                    series->attachAxis(axisY);
                }

                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignBottom);

                chartView1->setChart(chart);
                chartView1->setRenderHint(QPainter::Antialiasing);
            }

            mBandingAkumulasiSeksiChartHash[seksi.id()] = chartView1;

            CustomChartView *chartView2 = new CustomChartView;
            chartView2->setBackgroundBrush(QBrush(Qt::white));

            {
                QBarSeries *barSeries = new QBarSeries;

                QBarCategoryAxis *axisX = new QBarCategoryAxis;
                for (int bulan=1; bulan <= 12; bulan++)
                    axisX->append(Common::toNamaBulan(bulan).left(3));

                QValueAxis *axisY = new QValueAxis;
                axisY->setTitleText("Rupiah (Milyar)");

                QChart *chart = new QChart;
                chart->setAnimationOptions(QChart::SeriesAnimations);
                chart->setMargins(QMargins(0, 0, 0, 0));

                QFont font = chart->titleFont();
                font.setBold(true);
                font.setPixelSize(14);
                chart->setTitle("Perbandingan Penerimaan Per Bulan");
                chart->setTitleFont(font);

                chart->addSeries(barSeries);

                chart->addAxis(axisX, Qt::AlignBottom);
                chart->addAxis(axisY, Qt::AlignLeft);

                foreach (QAbstractSeries *series, chart->series()) {
                    series->attachAxis(axisX);
                    series->attachAxis(axisY);
                }

                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignBottom);

                chartView2->setChart(chart);
                chartView2->setRenderHint(QPainter::Antialiasing);
            }

            QHBoxLayout *waskonLayout = new QHBoxLayout;
            waskonLayout->setMargin(0);
            waskonLayout->addWidget(chartView1);
            waskonLayout->addWidget(chartView2);
            waskonLayout->addWidget(dialer);
            seksiLayout->addLayout(waskonLayout);

            mBandingBulanSeksiChartHash[seksi.id()] = chartView2;
        }

        QWidget *seksiWidget = new QWidget;
        seksiWidget->setLayout(seksiLayout);
        seksiWidget->setMinimumHeight(seksiPageHeight);
        seksiWidget->setAutoFillBackground(true);
        seksiWidget->setBackgroundRole(QPalette::Light);

        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setStyleSheet("QScrollArea { border: none; }");
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(seksiWidget);

        mSeksiStackedWidget->addWidget(scrollArea);
    }

    foreach (int seksi, seksiIdList)
    {
        QHash<int, double> mpnBulanCurrentYearHash;
        QHash<int, double> spmBulanCurrentYearHash;
        QHash<int, double> mpnBulanLastYearHash;
        QHash<int, double> spmBulanLastYearHash;
        QHash<int, double> spmkpBulanCurrentYearHash;
        QHash<int, double> spmkpBulanLastYearHash;
        QHash<int, double> spmppBulanCurrentYearHash;
        QHash<int, double> spmppBulanLastYearHash;
        QHash<int, double> pbkBulanCurrentYearHash;
        QHash<int, double> pbkBulanLastYearHash;
        QHash<int, double> renpenCurrentYearHash;

        double totalPenerimaanMpn(0);
        double totalPenerimaanSpm(0);
        double totalPenerimaan(0);
        double totalSpmkp(0);
        double totalSpmpp(0);
        double totalPbk(0);
        double totalRenpen(0);
        double totalNettoTahunLalu(0);

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalMpnBulanCurrentYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    mpnBulanCurrentYearHash[nominalIterator.key()] += nominalIterator.value();
                    totalPenerimaanMpn += nominalIterator.value();
                    totalPenerimaan += nominalIterator.value();
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalSpmBulanCurrentYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    spmBulanCurrentYearHash[nominalIterator.key()] += nominalIterator.value();
                    totalPenerimaanSpm += nominalIterator.value();
                    totalPenerimaan += nominalIterator.value();
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalSpmkpCurrentYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    spmkpBulanCurrentYearHash[nominalIterator.key()] += nominalIterator.value();
                    totalSpmkp += nominalIterator.value();
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalSpmppCurrentYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    spmppBulanCurrentYearHash[nominalIterator.key()] += nominalIterator.value();
                    totalSpmpp += nominalIterator.value();
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalPbkCurrentYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    pbkBulanCurrentYearHash[nominalIterator.key()] += nominalIterator.value();
                    totalPbk += nominalIterator.value();
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalMpnBulanLastYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    mpnBulanLastYearHash[nominalIterator.key()] += nominalIterator.value();

                    if (year == currentDate.year())
                    {
                        if (nominalIterator.key() <= currentDate.month())
                            totalNettoTahunLalu += nominalIterator.value();
                    }
                    else
                    {
                        totalNettoTahunLalu += nominalIterator.value();
                    }
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalSpmBulanLastYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    spmBulanLastYearHash[nominalIterator.key()] += nominalIterator.value();

                    if (year == currentDate.year())
                    {
                        if (nominalIterator.key() <= currentDate.month())
                            totalNettoTahunLalu += nominalIterator.value();
                    }
                    else
                    {
                        totalNettoTahunLalu += nominalIterator.value();
                    }
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalSpmkpLastYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    spmkpBulanLastYearHash[nominalIterator.key()] += nominalIterator.value();

                    if (year == currentDate.year())
                    {
                        if (nominalIterator.key() <= currentDate.month())
                            totalNettoTahunLalu -= nominalIterator.value();
                    }
                    else
                    {
                        totalNettoTahunLalu -= nominalIterator.value();
                    }
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalSpmppLastYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    spmppBulanLastYearHash[nominalIterator.key()] += nominalIterator.value();

                    if (year == currentDate.year())
                    {
                        if (nominalIterator.key() <= currentDate.month())
                            totalNettoTahunLalu -= nominalIterator.value();
                    }
                    else
                    {
                        totalNettoTahunLalu -= nominalIterator.value();
                    }
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalPbkLastYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    pbkBulanLastYearHash[nominalIterator.key()] += nominalIterator.value();

                    if (year == currentDate.year())
                    {
                        if (nominalIterator.key() <= currentDate.month())
                            totalNettoTahunLalu += nominalIterator.value();
                    }
                    else
                    {
                        totalNettoTahunLalu += nominalIterator.value();
                    }
                }
            }
        }

        {
            QHashIterator<int, QHash<int, double> > iterator(mTotalRenpenCurrentYearHash);
            while (iterator.hasNext())
            {
                iterator.next();
                if (seksi != 0 && iterator.key() != seksi)
                    continue;

                QHashIterator<int, double> nominalIterator(iterator.value());
                while (nominalIterator.hasNext())
                {
                    nominalIterator.next();
                    renpenCurrentYearHash[nominalIterator.key()] += nominalIterator.value();
                    totalRenpen += nominalIterator.value();
                }
            }
        }

        PenerimaanDialer *dialer = 0;
        QChartView *akumulasiChart = 0;
        QChartView *bulanChart = 0;

        if (seksi == 0)
        {
            dialer = mDialer;
            akumulasiChart = mBandingAkumulasiPenerimaanChart;
            bulanChart = mRasioMpnSpmChart;
        }
        else
        {
            dialer = mDialerSeksiHash[seksi];
            akumulasiChart = mBandingAkumulasiSeksiChartHash[seksi];
            bulanChart = mBandingBulanSeksiChartHash[seksi];
        }

        dialer->setPenerimaan(totalPenerimaan);
        dialer->setSpmkp(totalSpmkp);
        dialer->setSpmpp(totalSpmpp);
        dialer->setPbk(totalPbk);
        dialer->setRenpen(totalRenpen);
        dialer->setNettoTahunlalu(totalNettoTahunLalu);

        if (seksi == 0)
        {
            dialer->setTitle(QString("Penerimaan Kantor Tahun %1").arg(year));
            markCapai = dialer->capaian();
            markRenpen = totalRenpen;

            akumulasiChart->chart()->series()[0]->setName(QString("Renpen %1").arg(year));
            akumulasiChart->chart()->series()[1]->setName(QString("Realisasi %1").arg(year));
            akumulasiChart->chart()->series()[2]->setName(QString("Realisasi %1").arg(year - 1));
        }

        double maxTotalNominalBulan(0);
        {
            QBarSeries *barSeries = static_cast<QBarSeries *>(bulanChart->chart()->series()[0]);
            barSeries->clear();

            QBarSet *mpnSet = new QBarSet("MPN");
            QBarSet *mpnLastSet = new QBarSet("MPN Lalu");
            QBarSet *spmSet = new QBarSet("SPM");
            QBarSet *spmLastSet = new QBarSet("SPM Lalu");
            QBarSet *spmkpSet = new QBarSet("SPMKP/PP");
            QBarSet *spmkpLastSet = new QBarSet("SPMKP/PP Lalu");

            for (int bulan=1; bulan<=12; bulan++) {
                double mpn = mpnBulanCurrentYearHash[bulan] / 1000000000.0;
                double spm = spmBulanCurrentYearHash[bulan] / 1000000000.0;
                double mpnLast = mpnBulanLastYearHash[bulan] / 1000000000.0;
                double spmLast = spmBulanLastYearHash[bulan] / 1000000000.0;
                double spmkp = (spmkpBulanCurrentYearHash[bulan] + spmppBulanCurrentYearHash[bulan]) / 1000000000.0;
                double spmkpLast = (spmkpBulanLastYearHash[bulan] + spmppBulanLastYearHash[bulan]) / 1000000000.0;

                if (maxTotalNominalBulan < mpn)
                    maxTotalNominalBulan = mpn;

                if (maxTotalNominalBulan < mpnLast)
                    maxTotalNominalBulan = mpnLast;

                if (maxTotalNominalBulan < spm)
                    maxTotalNominalBulan = spm;

                if (maxTotalNominalBulan < spmLast)
                    maxTotalNominalBulan = spmLast;

                if (maxTotalNominalBulan < spmkp)
                    maxTotalNominalBulan = spmkp;

                if (maxTotalNominalBulan < spmkpLast)
                    maxTotalNominalBulan = spmkpLast;

                mpnSet->append(mpn);
                mpnLastSet->append(mpnLast);
                spmSet->append(spm);
                spmLastSet->append(spmLast);
                spmkpSet->append(spmkp);
                spmkpLastSet->append(spmkpLast);
            }

            barSeries->append(mpnSet);
            barSeries->append(mpnLastSet);
            barSeries->append(spmSet);
            barSeries->append(spmLastSet);
            barSeries->append(spmkpSet);
            barSeries->append(spmkpLastSet);

            QValueAxis *axis = static_cast<QValueAxis *>(bulanChart->chart()->axes(Qt::Vertical)[0]);
            axis->setRange(0, (maxTotalNominalBulan? maxTotalNominalBulan : 1));

            if (seksi == 0) {
                QChart *chart = bulanChart->chart();

                QLineSeries *testSeries = new QLineSeries;
                chart->addSeries(testSeries);
                QPointF zeroPos = chart->mapToPosition(QPoint(0,0), testSeries);
                chart->removeSeries(testSeries);

                chart->legend()->setPos(zeroPos.x() + 10, 45);
                chart->legend()->setVisible(true);
            }
        }

        double maxTotalNominalTahunan(0);
        {
            QLineSeries *renpenSeries = static_cast<QLineSeries *>(akumulasiChart->chart()->series()[0]);
            QLineSeries *penerimaanSeries = static_cast<QLineSeries *>(akumulasiChart->chart()->series()[1]);
            QLineSeries *penerimaanLastSeries = static_cast<QLineSeries *>(akumulasiChart->chart()->series()[2]);

            renpenSeries->clear();
            penerimaanSeries->clear();
            penerimaanLastSeries->clear();

            renpenSeries->append(0, 0);
            penerimaanSeries->append(0, 0);
            penerimaanLastSeries->append(0, 0);

            double totalRenpen(0);
            for (int b=1; b<=12; b++) {
                totalRenpen += renpenCurrentYearHash[b] / 1000000000.0;
                renpenSeries->append(b, totalRenpen);
            }

            if (maxTotalNominalTahunan < totalRenpen)
                maxTotalNominalTahunan = totalRenpen;

            for (int i=0; i<=1; i++) {
                double totalNominal(0);
                for (int b=1; b<=12; b++) {
                    if (i == 0) {
                        if (b <= currentDate.month() || year < currentDate.year()) {
                            totalNominal += (mpnBulanCurrentYearHash[b] + spmBulanCurrentYearHash[b] - (spmkpBulanCurrentYearHash[b] + spmppBulanCurrentYearHash[b]) + pbkBulanCurrentYearHash[b]) / 1000000000.0;
                            penerimaanSeries->append(b, totalNominal);
                        }
                    }
                    else {
                        totalNominal += (mpnBulanLastYearHash[b] + spmBulanLastYearHash[b] - (spmkpBulanLastYearHash[b] + spmppBulanCurrentYearHash[b]) + pbkBulanLastYearHash[b]) / 1000000000.0;
                        penerimaanLastSeries->append(b, totalNominal);
                    }
                }

                if (maxTotalNominalTahunan < totalNominal)
                    maxTotalNominalTahunan = totalNominal;
            }

            QValueAxis *axis = static_cast<QValueAxis *>(akumulasiChart->chart()->axes(Qt::Vertical)[0]);
            axis->setRange(0, (maxTotalNominalTahunan? maxTotalNominalTahunan : 1));

            if (seksi == 0) {
                QChart *chart = akumulasiChart->chart();
                QPointF zeroPos = chart->mapToPosition(QPoint(0,0));
                chart->legend()->setPos(zeroPos.x() + 10, 45);
                chart->legend()->setVisible(true);
            }
        }

        {
            QString namaSeksi;
            if (seksi != 0)
                namaSeksi = Seksi::getSeksi(seksi, mKantor.kode()).nama();

            {
                QBrush headerBackground(QColor(0, 128, 255, 50));
                QBrush contentBackground(QColor(233, 233, 233, 60));

                QFont tableHeaderFont = mResultView->font();
                tableHeaderFont.setBold(true);

                QFont headerFont = tableHeaderFont;
                headerFont.setPixelSize(14);

                angkaRow++;
                {
                    QString title = "Perbandingan Akumulasi Penerimaan";
                    if (seksi == 0)
                        title += " Kantor";
                    else
                        title += " Seksi";

                    QStandardItem *titleItem = new QStandardItem(title);
                    titleItem->setTextAlignment(Qt::AlignCenter);
                    titleItem->setFont(headerFont);
                    model->setItem(angkaRow, 1, titleItem);

                    QHash<QString, int> header;
                    header["row"] = angkaRow;
                    header["span"] = 7;
                    angkaHeaderList << header;

                    if (seksi != 0)
                    {
                        QStandardItem *seksiItem = new QStandardItem(namaSeksi);
                        seksiItem->setTextAlignment(Qt::AlignCenter);
                        seksiItem->setFont(headerFont);
                        model->setItem(++angkaRow, 1, seksiItem);

                        QHash<QString, int> header;
                        header["row"] = angkaRow;
                        header["span"] = 7;
                        angkaHeaderList << header;
                    }

                    angkaRow += 2;
                    for (int i=0; i<7; i++)
                    {
                        QString title;
                        if (i == 0) title = "Bulan";
                        else if (i == 1) title = QString("Target");
                        else if (i == 2) title = QString("Netto Tahun %1").arg(year - 1);
                        else if (i == 3) title = QString("Netto Tahun %1").arg(year);
                        else if (i == 4) title = QString("Selisih");
                        else if (i == 5) title = QString("Capai");
                        else title = "Tumbuh";

                        QStandardItem *headerItem = new QStandardItem(title);
                        headerItem->setTextAlignment(Qt::AlignCenter);
                        headerItem->setFont(tableHeaderFont);
                        headerItem->setBackground(headerBackground);
                        model->setItem(angkaRow, 1+i, headerItem);
                    }

                    double totalRenpen(0), totalLalu(0), totalCurrent(0), selisih(0);
                    for (int b=1; b<=12; b++)
                    {
                        angkaRow++;
                        totalRenpen += renpenCurrentYearHash[b];
                        totalLalu += mpnBulanLastYearHash[b] + spmBulanLastYearHash[b] - (spmkpBulanLastYearHash[b] + spmppBulanLastYearHash[b]) + pbkBulanLastYearHash[b];
                        totalCurrent += mpnBulanCurrentYearHash[b] + spmBulanCurrentYearHash[b] - (spmkpBulanCurrentYearHash[b] + spmppBulanCurrentYearHash[b]) + pbkBulanCurrentYearHash[b];
                        selisih = totalCurrent - totalLalu;

                        float capai(0);
                        if (totalRenpen)
                            capai = totalCurrent / totalRenpen;

                        float tumbuh(0);
                        if (totalLalu)
                            tumbuh = selisih / qFabs(totalLalu);
                        else
                            tumbuh = selisih / qFabs(selisih);

                        QStandardItem *bulanItem = new QStandardItem(Common::toNamaBulan(b));
                        bulanItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 1, bulanItem);

                        QStandardItem *targetItem = new QStandardItem;
                        targetItem->setData(totalRenpen, Qt::DisplayRole);
                        targetItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        targetItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 2, targetItem);

                        QStandardItem *lastItem = new QStandardItem;
                        lastItem->setData(totalLalu, Qt::DisplayRole);
                        lastItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        lastItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 3, lastItem);

                        QStandardItem *currentItem = new QStandardItem;
                        currentItem->setData(totalCurrent, Qt::DisplayRole);
                        currentItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        currentItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 4, currentItem);

                        QStandardItem *selisihItem = new QStandardItem;
                        selisihItem->setData(selisih, Qt::DisplayRole);
                        selisihItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        selisihItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 5, selisihItem);

                        QStandardItem *capaiItem = new QStandardItem;
                        capaiItem->setData(capai, Qt::DisplayRole);
                        capaiItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        capaiItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 6, capaiItem);

                        QStandardItem *tumbuhItem = new QStandardItem;
                        tumbuhItem->setData(tumbuh, Qt::DisplayRole);
                        tumbuhItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        tumbuhItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 7, tumbuhItem);
                    }
                }

                angkaRow += 2;
                {
                    QString title = "Perbandingan Penerimaan Per Bulan";
                    if (seksi == 0)
                        title += " Kantor";
                    else
                        title += " Seksi";

                    QStandardItem *titleItem = new QStandardItem(title);
                    titleItem->setTextAlignment(Qt::AlignCenter);
                    titleItem->setFont(headerFont);
                    model->setItem(angkaRow, 1, titleItem);

                    QHash<QString, int> header;
                    header["row"] = angkaRow;
                    header["span"] = 7;
                    angkaHeaderList << header;

                    if (seksi != 0)
                    {
                        QStandardItem *seksiItem = new QStandardItem(namaSeksi);
                        seksiItem->setTextAlignment(Qt::AlignCenter);
                        seksiItem->setFont(headerFont);
                        model->setItem(++angkaRow, 1, seksiItem);

                        QHash<QString, int> header;
                        header["row"] = angkaRow;
                        header["span"] = 7;
                        angkaHeaderList << header;
                    }

                    angkaRow += 2;
                    for (int i=0; i<7; i++)
                    {
                        QString title;
                        if (i == 0) title = "Bulan";
                        else if (i == 1) title = QString("Target");
                        else if (i == 2) title = QString("Netto Tahun %1").arg(year - 1);
                        else if (i == 3) title = QString("Netto Tahun %1").arg(year);
                        else if (i == 4) title = QString("Selisih");
                        else if (i == 5) title = QString("Capai");
                        else title = "Tumbuh";

                        QStandardItem *headerItem = new QStandardItem(title);
                        headerItem->setTextAlignment(Qt::AlignCenter);
                        headerItem->setFont(tableHeaderFont);
                        headerItem->setBackground(headerBackground);
                        model->setItem(angkaRow, 1+i, headerItem);
                    }

                    double totalLalu(0), totalCurrent(0), selisih(0);
                    for (int b=1; b<=12; b++)
                    {
                        ++angkaRow;

                        totalLalu = mpnBulanLastYearHash[b] + spmBulanLastYearHash[b] - (spmkpBulanLastYearHash[b] + spmppBulanLastYearHash[b]) + pbkBulanLastYearHash[b];
                        totalCurrent = mpnBulanCurrentYearHash[b] + spmBulanCurrentYearHash[b] - (spmkpBulanCurrentYearHash[b] + spmppBulanCurrentYearHash[b]) + pbkBulanCurrentYearHash[b];
                        selisih = totalCurrent - totalLalu;

                        float capai(0);
                        if (renpenCurrentYearHash[b])
                            capai = totalCurrent / renpenCurrentYearHash[b];

                        float tumbuh(0);
                        if (totalLalu)
                            tumbuh = selisih / qFabs(totalLalu);
                        else
                            tumbuh = selisih / qFabs(selisih);

                        QStandardItem *bulanItem = new QStandardItem(Common::toNamaBulan(b));
                        bulanItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 1, bulanItem);

                        QStandardItem *targetItem = new QStandardItem;
                        targetItem->setData(renpenCurrentYearHash[b], Qt::DisplayRole);
                        targetItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        targetItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 2, targetItem);

                        QStandardItem *lastItem = new QStandardItem;
                        lastItem->setData(totalLalu, Qt::DisplayRole);
                        lastItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        lastItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 3, lastItem);

                        QStandardItem *currentItem = new QStandardItem;
                        currentItem->setData(totalCurrent, Qt::DisplayRole);
                        currentItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        currentItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 4, currentItem);

                        QStandardItem *selisihItem = new QStandardItem;
                        selisihItem->setData(selisih, Qt::DisplayRole);
                        selisihItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        selisihItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 5, selisihItem);

                        QStandardItem *capaiItem = new QStandardItem;
                        capaiItem->setData(capai, Qt::DisplayRole);
                        capaiItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        capaiItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 6, capaiItem);

                        QStandardItem *tumbuhItem = new QStandardItem;
                        tumbuhItem->setData(tumbuh, Qt::DisplayRole);
                        tumbuhItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        tumbuhItem->setBackground(contentBackground);
                        model->setItem(angkaRow, 7, tumbuhItem);
                    }
                }

                angkaRow++;
            }
        }
    }

    QStandardItem *hopeItem = new QStandardItem("Terima kasih untuk do'a dan supportnya selama ini...");
    model->setItem(model->rowCount() -1, model->columnCount() -3, hopeItem);

    mResultView->setModel(model);
    QListIterator<QHash<QString,int> > headerIterator(angkaHeaderList);
    while (headerIterator.hasNext())
    {
        const QHash<QString,int> &value = headerIterator.next();
        mResultView->setSpan(value["row"], 1, 1, value["span"]);
    }

    mResultView->setSpan(model->rowCount() -1, model->columnCount() -3, 1, 3);

    mResultView->setColumnWidth(1, 100);
    mResultView->setColumnWidth(2, 160);
    mResultView->setColumnWidth(3, 160);
    mResultView->setColumnWidth(4, 160);
    mResultView->setColumnWidth(5, 160);
    mResultView->setColumnWidth(6, 100);
    mResultView->setColumnWidth(7, 100);

    MpnInfoEngine *engine = static_cast<MpnInfoEngine *>(mEngine);
    mLastMpnLabel->setText(QString("Data MPN: ") + engine->lastMpn().toString("dd-MM-yyyy"));
    mLastSpmLabel->setText(QString("Data SPM: ") + engine->lastSpm().toString("dd-MM-yyyy"));

    QString version = QString("MPN-Info v%1").arg(IDD_PACKAGE_VERSION);
    mVersionLabel->setText(version);

    mKantorComboBox->setEnabled(true);
    mYearComboBox->setEnabled(true);

    {
        QString kodeKantor = mEngine->serverSetting(IDS_SERVER_KANTOR_KODE).toString();
        int currentYear = QDate::currentDate().year();
        if (markCapai >= 100
                && markRenpen > 0
                && kodeKantor == mKantorComboBox->currentData().toString()
                && currentYear == mYearComboBox->currentData().toInt())
            mStackedWidget->setCurrentIndex(2);
        else
            mStackedWidget->setCurrentIndex(1);
    }

    mTotalMpnBulanCurrentYearHash.clear();
    mTotalMpnBulanLastYearHash.clear();
    mTotalSpmBulanCurrentYearHash.clear();
    mTotalSpmBulanLastYearHash.clear();
    mTotalSpmkpCurrentYearHash.clear();
    mTotalSpmkpLastYearHash.clear();
    mTotalSpmppCurrentYearHash.clear();
    mTotalSpmppLastYearHash.clear();
    mTotalPbkCurrentYearHash.clear();
    mTotalPbkLastYearHash.clear();
    mTotalRenpenCurrentYearHash.clear();
}

void HomePage::onWinnerButtonClicked()
{
    mStackedWidget->setCurrentIndex(1);
}
