#include "homepage.h"
#include "homepage_p.h"

#include <qmath.h>
#include <QComboBox>
#include <QTimer>
#include <QGridLayout>
#include <QSqlQuery>
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QScrollArea>
#include <QPalette>

#include "customtableview.h"
#include "dialer.h"
#include "penerimaandialer.h"

HomePage::HomePage(CoreEngine *engine, QWidget *parent) :
    Page(parent),
    mEngine(engine)
{
    HomeThread *homeThread = new HomeThread(mEngine, this);
    mThread = homeThread;

    mKantorComboBox = new QComboBox;
    QString kodeKantor = mEngine->serverSetting(IDS_SERVER_KANTOR_KODE).toString();
    Kantor kantor = Kantor::getKantor(kodeKantor);
    foreach (const Kantor &kantor, Kantor::getKantorKanwilList(kodeKantor))
        mKantorComboBox->addItem(kantor.nama(), kantor.kode());

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

    QVector<double> bulanTicks;
    QVector<QString> bulanLabels;
    for (int bulan=1; bulan <= 12; bulan++) {
        bulanTicks << bulan;
        bulanLabels << Common::toNamaBulan(bulan);
    }

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "Mei" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Des";

    // Total Penerimaan Bulan
    mBandingAkumulasiPenerimaanPlot = new Highcharts::Charts;
    mBandingAkumulasiPenerimaanPlot->setTitle("Perbandingan Akumulasi Penerimaan");
    mBandingAkumulasiPenerimaanPlot->setAxisTitle(Qt::Vertical, "Nominal (Milyar)");
    mBandingAkumulasiPenerimaanPlot->setCategories(categories);

    // Dialer
    mDialer = new PenerimaanDialer;

    // Rasio MPN dan SPM
    mRasioMpnSpmPlot = new Highcharts::Charts;
    mRasioMpnSpmPlot->setTitle("Perbandingan Penerimaan Per Bulan");
    mRasioMpnSpmPlot->setAxisTitle(Qt::Vertical, "Nominal (Milyar)");
    mRasioMpnSpmPlot->setColumnStackingEnabled(true);
    mRasioMpnSpmPlot->setCategories(categories);
    mRasioMpnSpmPlot->set3DEnabled(true);
    mRasioMpnSpmPlot->set3DAlpha(15);
    mRasioMpnSpmPlot->set3DBeta(15);
    mRasioMpnSpmPlot->set3DViewDistance(25);
    mRasioMpnSpmPlot->set3DDepth(40);

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
    kantorLayout->addWidget(mBandingAkumulasiPenerimaanPlot, 1, 1);
    kantorLayout->addWidget(mRasioMpnSpmPlot, 2, 1);

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

    connect(homeThread, SIGNAL(gotData(QVariantMap)), SLOT(gotData(QVariantMap)));
    connect(homeThread, SIGNAL(finished()), SLOT(updateResult()));
    connect(mKantorComboBox, SIGNAL(currentIndexChanged(int)), SLOT(refresh()));
    connect(mYearComboBox, SIGNAL(currentIndexChanged(int)), SLOT(refresh()));

    refresh();
}

void HomePage::refresh()
{
    delete mResultView->model();
    mPegawaiHash.clear();
    mWajibPajakHash.clear();

    mDialerSeksiHash.clear();
    mPlotBandingAkumulasiSeksiHash.clear();
    mPlotBandingBulanSeksiHash.clear();

    QWidget *seksiWidget = mSeksiStackedWidget->widget(0);
    mSeksiStackedWidget->removeWidget(seksiWidget);
    delete seksiWidget;

    QString kodeKantor = mKantorComboBox->currentData().toString();
    mPegawaiHash = Pegawai::getPegawaiHash(kodeKantor, mYearComboBox->currentData().toInt());

    HomeThread *thread = static_cast<HomeThread *>(mThread);
    thread->setKantor(mKantorComboBox->currentData().toString());
    thread->setTahun(mYearComboBox->currentData().toInt());

    mKantorComboBox->setEnabled(false);
    mYearComboBox->setEnabled(false);

    mStackedWidget->setCurrentIndex(0);
    thread->start();
}

void HomePage::gotData(const QVariantMap &data)
{
    int type = data["type"].toInt();
    QString npwp = data["npwp"].toString();
    QString kpp = data["kpp"].toString();
    QString cabang = data["cabang"].toString();
    QString nip = data["nip"].toString();
    QString sektor = data["sektor"].toString();
    int bulan = data["bulan"].toInt();
    double nominal = data["nominal"].toDouble();

    switch (type)
    {
    case HomeThread::DataWajibPajak:
    {
        WajibPajak wp;
        wp.setSektor(sektor);
        wp.setNipPj(nip);

        mWajibPajakHash[npwp + kpp + cabang] = wp;
    }
        break;
    default:
        break;
    }

    int seksi(0);
    switch (type)
    {
    case HomeThread::DataMpnCurrentYear:
    case HomeThread::DataMpnLastYear:
    case HomeThread::DataSpmCurrentYear:
    case HomeThread::DataSpmLastYear:
    case HomeThread::DataSpmkpCurrentYear:
    case HomeThread::DataSpmppCurrentYear:
        seksi = mPegawaiHash[mWajibPajakHash[npwp + kpp + cabang].nipPj()].seksi();
        break;
    case HomeThread::DataRenpenCurrentYear:
        seksi = mPegawaiHash[nip].seksi();
        break;
    default:
        break;
    }

    switch (type)
    {
    case HomeThread::DataMpnCurrentYear:
        mTotalMpnBulanCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataMpnLastYear:
        mTotalMpnBulanLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataSpmCurrentYear:
        mTotalSpmBulanCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataSpmLastYear:
        mTotalSpmBulanLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataSpmkpCurrentYear:
        mTotalSpmkpCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataSpmkpLastYear:
        mTotalSpmkpLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataSpmppCurrentYear:
        mTotalSpmppCurrentYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataSpmppLastYear:
        mTotalSpmppLastYearHash[seksi][bulan] += nominal;
        break;
    case HomeThread::DataRenpenCurrentYear:
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

    Kantor kantor = Kantor::getKantor(mKantorComboBox->currentData().toString());
    if (kantor.isKanwil())
    {
        mTabWidget->setTabEnabled(1, false);
    }
    else
    {
        mTabWidget->setTabEnabled(1, true);

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

        QList<Seksi> seksiList = Seksi::getSeksiList(kantor.kode(), tipeSeksiList);
        seksiPageHeight += 400 * seksiList.count();

        foreach (const Seksi &seksi, seksiList)
        {
            seksiIdList << seksi.id();

            QStringList categories;
            categories << "Jan" << "Feb" << "Mar" << "Apr" << "Mei" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Des";

            PenerimaanDialer *dialer = new PenerimaanDialer;
            dialer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            dialer->setTitle(QString("Penerimaan Seksi\n%1").arg(seksi.nama()));

            mDialerSeksiHash[seksi.id()] = dialer;

            Highcharts::Charts *plot = new Highcharts::Charts;
            plot->setTitle(QString("Perbandingan Akumulasi Penerimaan<br />Seksi %1").arg(seksi.nama()));
            plot->setAxisTitle(Qt::Vertical, "Nominal (Milyar)");
            plot->setCategories(categories);

            mPlotBandingAkumulasiSeksiHash[seksi.id()] = plot;

            Highcharts::Charts *plot1 = new Highcharts::Charts;
            plot1->setTitle(QString("Perbandingan Penerimaan Per Bulan<br />Seksi %1").arg(seksi.nama()));
            plot1->setAxisTitle(Qt::Vertical, "Nominal (Milyar)");
            plot1->setCategories(categories);

            QHBoxLayout *waskonLayout = new QHBoxLayout;
            waskonLayout->setMargin(0);
            waskonLayout->addWidget(plot);
            waskonLayout->addWidget(plot1);
            waskonLayout->addWidget(dialer);
            seksiLayout->addLayout(waskonLayout);

            mPlotBandingBulanSeksiHash[seksi.id()] = plot1;
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
        QHash<int, double> renpenCurrentYearHash;

        double totalPenerimaanMpn(0);
        double totalPenerimaanSpm(0);
        double totalPenerimaan(0);
        double totalSpmkp(0);
        double totalSpmpp(0);
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
        Highcharts::Charts *akumulasiPlot = 0;
        Highcharts::Charts *bulanPlot = 0;

        if (seksi == 0)
        {
            dialer = mDialer;
            akumulasiPlot = mBandingAkumulasiPenerimaanPlot;
            bulanPlot = mRasioMpnSpmPlot;
        }
        else
        {
            dialer = mDialerSeksiHash[seksi];
            akumulasiPlot = mPlotBandingAkumulasiSeksiHash[seksi];
            bulanPlot = mPlotBandingBulanSeksiHash[seksi];
        }

        dialer->setPenerimaan(totalPenerimaan);
        dialer->setSpmkp(totalSpmkp);
        dialer->setSpmpp(totalSpmpp);
        dialer->setRenpen(totalRenpen);
        dialer->setNettoTahunlalu(totalNettoTahunLalu);

        if (seksi == 0)
        {
            dialer->setTitle(QString("Penerimaan Kantor Tahun %1").arg(year));
            markCapai = dialer->capaian();
            markRenpen = totalRenpen;
        }

        // rasio MPN SPM
        {
            Highcharts::Series lastSeries;
            lastSeries.setName(QString("Total %1").arg(year-1));
            lastSeries.setStackName("lastYear");

            Highcharts::Series currentSeries;
            currentSeries.setName(QString("Total %1").arg(year));
            currentSeries.setStackName("currentYear");

            Highcharts::Series mpnSeries;
            mpnSeries.setName(QString("MPN %1").arg(year));
            mpnSeries.setStackName("currentYear");

            Highcharts::Series spmSeries;
            spmSeries.setName(QString("SPM %1").arg(year));
            spmSeries.setStackName("currentYear");

            if (seksi == 0) {
                lastSeries.setType(Highcharts::Column);
                mpnSeries.setType(Highcharts::Column);
                spmSeries.setType(Highcharts::Column);
            }

            for (int bulan=1; bulan<=12; bulan++) {
                double mpn = mpnBulanCurrentYearHash[bulan] / 1000000000.0;
                double spm = spmBulanCurrentYearHash[bulan] / 1000000000.0;
                double mpnLast = mpnBulanLastYearHash[bulan] / 1000000000.0;
                double spmLast = spmBulanLastYearHash[bulan] / 1000000000.0;

                lastSeries.addData(Highcharts::Data(mpnLast + spmLast));
                currentSeries.addData(Highcharts::Data((mpn) + spm));
                mpnSeries.addData(Highcharts::Data(mpn));
                spmSeries.addData(Highcharts::Data(spm));
            }

            bulanPlot->addSeries(lastSeries);
            if (seksi == 0) {
                bulanPlot->addSeries(mpnSeries);
                bulanPlot->addSeries(spmSeries);
            }
            else {
                bulanPlot->addSeries(currentSeries);
            }
        }
        bulanPlot->render();

        for (int i=1; i>=0; i--) {
            Highcharts::Series series;
            series.setName(QString("Tahun %1").arg(year-i));

            double totalNominal = 0;
            for (int b=1; b<=12; b++) {
                if (i == 0)
                    totalNominal += (mpnBulanCurrentYearHash[b] + spmBulanCurrentYearHash[b] - (spmkpBulanCurrentYearHash[b] + spmppBulanCurrentYearHash[b])) / 1000000000.0;
                else
                    totalNominal += (mpnBulanLastYearHash[b] + spmBulanLastYearHash[b] - (spmkpBulanLastYearHash[b] + spmppBulanCurrentYearHash[b])) / 1000000000.0;

                series.addData(Highcharts::Data(totalNominal));

                if (i == 0 && year == currentDate.year() && b == currentDate.month())
                    break;
            }

            akumulasiPlot->addSeries(series);
        }
        akumulasiPlot->render();

        {
            QString namaSeksi;
            if (seksi != 0)
                namaSeksi = Seksi::getSeksi(seksi, kantor.kode()).nama();

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
                        totalLalu += mpnBulanLastYearHash[b] + spmBulanLastYearHash[b] - (spmkpBulanLastYearHash[b] + spmppBulanLastYearHash[b]);
                        totalCurrent += mpnBulanCurrentYearHash[b] + spmBulanCurrentYearHash[b] - (spmkpBulanCurrentYearHash[b] + spmppBulanCurrentYearHash[b]);
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

                        totalLalu = mpnBulanLastYearHash[b] + spmBulanLastYearHash[b] - (spmkpBulanLastYearHash[b] + spmppBulanLastYearHash[b]);
                        totalCurrent = mpnBulanCurrentYearHash[b] + spmBulanCurrentYearHash[b] - (spmkpBulanCurrentYearHash[b] + spmppBulanCurrentYearHash[b]);
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

    QStandardItem *hopeItem = new QStandardItem("Doakan saya cepat pindah dekat keluarga ya... :'(");
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
    mTotalRenpenCurrentYearHash.clear();
}

void HomePage::onWinnerButtonClicked()
{
    mStackedWidget->setCurrentIndex(1);
}
