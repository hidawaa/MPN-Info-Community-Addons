#include "masterfilesearchpage.h"
#include "masterfilesearch_p.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLineEdit>
#include <QDateEdit>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QApplication>
#include <QIcon>
#include <QDate>
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QMimeData>
#include <QThread>
#include <QKeyEvent>

#include "xlsxdocument.h"

#include <coredata.h>
#include <enums.h>
#include <QStyledItemDelegate>
#include "kluselectdialog.h"
#include <QPushButton>

class DateDelegate : public QStyledItemDelegate
{
public:
    explicit DateDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QString displayText(const QVariant &value, const QLocale &locale) const override
    {
        if (value.typeId() == QMetaType::QDate) {
            QDate d = value.toDate();
            if (d.isValid())
                return d.toString("dd-MM-yyyy");
            return "";
        }
        return QStyledItemDelegate::displayText(value, locale);
    }
};

MasterfileSearchPage::MasterfileSearchPage(CoreEngine *engine, QWidget *parent)
    : Page(parent), mEngine(engine)
{
    mFunction = new MasterfileSearchFunction(mEngine, this);
    connect(mFunction, SIGNAL(rowsReady(QList<QVariantList>)), SLOT(appendRows(QList<QVariantList>)));
    connect(mFunction, SIGNAL(finished(int)), SLOT(searchFinished(int)));

    mResultView = new QTableView;
    mResultView->horizontalHeader()->setStretchLastSection(false);
    mResultView->verticalHeader()->hide();
    
    QFont tableFont = mResultView->font();
    tableFont.setPointSize(8);
    mResultView->setFont(tableFont);
    mResultView->setAlternatingRowColors(true);
    mResultView->setWordWrap(false);
    mResultView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mResultView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mResultView->setSortingEnabled(true);
    mResultView->verticalHeader()->setDefaultSectionSize(mResultView->fontMetrics().height() + 10);
    mResultView->horizontalHeader()->setHighlightSections(false);
    mResultView->verticalHeader()->setHighlightSections(false);

    mKantorComboBox = new QComboBox;
    mKantorComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QString kodeKantor = mEngine->databaseSettings()->value("kantor.kode").toString();
    Kantor kantor = mEngine->data()->kantor(kodeKantor);
    
    if (kantor.type == Kanwil) {
        foreach (const Kantor &tempKantor, mEngine->data()->kppList(kantor.kanwil))
            mKantorComboBox->addItem(tempKantor.nama, tempKantor.kode);
    } else {
        mKantorComboBox->addItem(kantor.nama, kantor.kode);
    }

    mNpwpCheckBox = new QCheckBox;
    mNpwpLineEdit = new QLineEdit;
    mNpwpLineEdit->setAlignment(Qt::AlignCenter);
    mNpwpLineEdit->setInputMask("00.000.000.0-000.000;_");
    mNpwpLineEdit->setMinimumWidth(130);
    mNpwpLineEdit->setMaximumWidth(130);
    mNpwpLineEdit->setEnabled(false);

    mNamaCheckBox = new QCheckBox;
    mNamaLineEdit = new QLineEdit;
    mNamaLineEdit->setEnabled(false);

    mKluCheckBox = new QCheckBox;
    mKluLineEdit = new QLineEdit;
    mKluLineEdit->setAlignment(Qt::AlignCenter);
    mKluLineEdit->setMaximumWidth(100);
    mKluLineEdit->setEnabled(false);

    mKluSelectButton = new QPushButton(QIcon(":/images/search.png"), "");
    mKluSelectButton->setFixedSize(22, 22);
    mKluSelectButton->setEnabled(false);

    QHBoxLayout *kluLayout = new QHBoxLayout;
    kluLayout->addWidget(mKluLineEdit);
    kluLayout->addWidget(mKluSelectButton);
    kluLayout->addStretch();

    mAlamatCheckBox = new QCheckBox;
    mAlamatLineEdit = new QLineEdit;
    mAlamatLineEdit->setEnabled(false);

    QHBoxLayout *alamatLayout = new QHBoxLayout;
    alamatLayout->addWidget(mAlamatLineEdit);

    mJenisWpCheckBox = new QCheckBox;
    mJenisWpComboBox = new QComboBox;
    mJenisWpComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mJenisWpComboBox->setEnabled(false);

    mJenisWpComboBox->addItem("Orang Pribadi", "OP");
    mJenisWpComboBox->addItem("Badan", "BADAN");
    mJenisWpComboBox->addItem("Pemungut", "PEMUNGUT");
    mJenisWpComboBox->addItem("Unknown", "");

    QHBoxLayout *jenisWpLayout = new QHBoxLayout;
    jenisWpLayout->addWidget(new QLabel("Jenis Wajib Pajak"));
    jenisWpLayout->addWidget(mJenisWpComboBox);

    mStatusWpCheckBox = new QCheckBox;
    mStatusWpComboBox = new QComboBox;
    mStatusWpComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mStatusWpComboBox->setEnabled(false);

    mStatusWpComboBox->addItem("Normal");
    mStatusWpComboBox->addItem("Non Efektif");
    mStatusWpComboBox->addItem("Pendaftaraan Baru");
    mStatusWpComboBox->addItem("Pindah Baru");

    QHBoxLayout *statusWpLayout = new QHBoxLayout;
    statusWpLayout->addWidget(new QLabel("Status Wajib Pajak"));
    statusWpLayout->addWidget(mStatusWpComboBox);

    mSeksiCheckBox = new QCheckBox;
    mSeksiComboBox = new QComboBox;
    mSeksiComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mSeksiComboBox->setEnabled(false);

    mArCheckBox = new QCheckBox;
    mPjComboBox = new QComboBox;
    mPjComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mPjComboBox->setEnabled(false);

    updateFilterKantor();
    if (kantor.type == Kanwil) {
        mSeksiCheckBox->setEnabled(false);
        mArCheckBox->setEnabled(false);
    }

    QDate currentDate = QDate::currentDate();

    mTglDaftarCheckBox = new QCheckBox;
    mTglDaftarStartDateEdit = new QDateEdit;
    mTglDaftarStartDateEdit->setMaximumWidth(120);
    mTglDaftarStartDateEdit->setAlignment(Qt::AlignCenter);
    mTglDaftarStartDateEdit->setDate(currentDate);
    mTglDaftarStartDateEdit->setMaximumDate(currentDate);
    mTglDaftarStartDateEdit->setDisplayFormat("dd-MM-yyyy");
    mTglDaftarStartDateEdit->setCalendarPopup(true);
    mTglDaftarStartDateEdit->setEnabled(false);

    mTglDaftarEndDateEdit = new QDateEdit;
    mTglDaftarEndDateEdit->setMaximumWidth(120);
    mTglDaftarEndDateEdit->setAlignment(Qt::AlignCenter);
    mTglDaftarEndDateEdit->setDate(currentDate);
    mTglDaftarEndDateEdit->setMaximumDate(currentDate);
    mTglDaftarEndDateEdit->setDisplayFormat("dd-MM-yyyy");
    mTglDaftarEndDateEdit->setCalendarPopup(true);
    mTglDaftarEndDateEdit->setEnabled(false);

    QHBoxLayout *tglDaftarLayoutStart = new QHBoxLayout;
    tglDaftarLayoutStart->addWidget(mTglDaftarStartDateEdit);
    tglDaftarLayoutStart->addStretch();

    QHBoxLayout *tglDaftarLayoutEnd = new QHBoxLayout;
    tglDaftarLayoutEnd->addWidget(mTglDaftarEndDateEdit);
    tglDaftarLayoutEnd->addStretch();

    QPushButton *searchButton = new QPushButton(QIcon(":/images/search.png"), "Search");
    QPushButton *saveButton = new QPushButton(QIcon(":/images/save.png"), "Save");

    QHBoxLayout *toolBox = new QHBoxLayout;
    toolBox->addStretch();
    toolBox->addWidget(searchButton);
    toolBox->addWidget(saveButton);

    QGridLayout *inputLayout = new QGridLayout;
    inputLayout->addWidget(new QLabel("Kantor"), 0, 1, 1, 2);
    inputLayout->addWidget(mKantorComboBox, 0, 3);
    inputLayout->addWidget(mNpwpCheckBox, 1, 0);
    inputLayout->addWidget(new QLabel("NPWP"), 1, 1, 1, 2);
    inputLayout->addWidget(mNpwpLineEdit, 1, 3);
    inputLayout->addWidget(mNamaCheckBox, 2, 0);
    inputLayout->addWidget(new QLabel("Nama"), 2, 1, 1, 2);
    inputLayout->addWidget(mNamaLineEdit, 2, 3);
    inputLayout->addWidget(mKluCheckBox, 3, 0);
    inputLayout->addWidget(new QLabel("KLU"), 3, 1, 1, 2);
    inputLayout->addLayout(kluLayout, 3, 3);
    inputLayout->addWidget(mAlamatCheckBox, 4, 0);
    inputLayout->addWidget(new QLabel("Alamat"), 4, 1, 1, 2);
    inputLayout->addLayout(alamatLayout, 4, 3);
    inputLayout->addWidget(mJenisWpCheckBox, 5, 0);
    inputLayout->addLayout(jenisWpLayout, 5, 1, 1, 3);
    inputLayout->addWidget(mStatusWpCheckBox, 6, 0);
    inputLayout->addLayout(statusWpLayout, 6, 1, 1, 3);
    inputLayout->addWidget(mTglDaftarCheckBox, 7, 0);
    inputLayout->addWidget(new QLabel("Tanggal Daftar"), 7, 1, 1, 3);
    inputLayout->addWidget(new QLabel("Start"), 8, 1);
    inputLayout->addLayout(tglDaftarLayoutStart, 8, 2, 1, 2);
    inputLayout->addWidget(new QLabel("End"), 9, 1);
    inputLayout->addLayout(tglDaftarLayoutEnd, 9, 2, 1, 2);
    inputLayout->addWidget(mSeksiCheckBox, 10, 0);
    inputLayout->addWidget(new QLabel("Seksi"), 10, 1, 1, 2);
    inputLayout->addWidget(mSeksiComboBox, 10, 3);
    inputLayout->addWidget(mArCheckBox, 11, 0);
    inputLayout->addWidget(new QLabel("PJ"), 11, 1, 1, 2);
    inputLayout->addWidget(mPjComboBox, 11, 3);

    QGroupBox *inputGroup = new QGroupBox("Filter Pencarian");
    inputGroup->setLayout(inputLayout);

    QVBoxLayout *sideLayout = new QVBoxLayout;
    sideLayout->addWidget(inputGroup);
    sideLayout->addLayout(toolBox);
    sideLayout->addStretch();
    sideLayout->setContentsMargins(0, 0, 0, 0);

    mStatusLabel = new QLabel("Tekan Search untuk memulai pencarian");

    QWidget *sidebar = new QWidget;
    sidebar->setMinimumWidth(250);
    sidebar->setMaximumWidth(250);
    sidebar->setLayout(sideLayout);
    sidebar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->addWidget(mResultView);
    contentLayout->addWidget(mStatusLabel);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(sidebar);
    layout->addLayout(contentLayout);

    setLayout(layout);
    setWindowTitle("Cari Data Wajib Pajak");

    connect(mKantorComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateFilter()));
    connect(mKantorComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateFilterKantor()));
    connect(mNpwpCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mNamaCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mKluCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mAlamatCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mJenisWpCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mStatusWpCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mSeksiCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mArCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    connect(mTglDaftarCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateFilter()));
    
    connect(mNpwpLineEdit, SIGNAL(returnPressed()), SLOT(search()));
    connect(mNamaLineEdit, SIGNAL(returnPressed()), SLOT(search()));
    connect(mKluLineEdit, SIGNAL(returnPressed()), SLOT(search()));
    
    connect(searchButton, SIGNAL(clicked()), this, SLOT(search()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(mKluSelectButton, SIGNAL(clicked()), this, SLOT(selectKlu()));

    QStandardItemModel *model = new QStandardItemModel(0, 17, mResultView);
    model->setHeaderData(0, Qt::Horizontal, "NPWP");
    model->setHeaderData(1, Qt::Horizontal, "Nama");
    model->setHeaderData(2, Qt::Horizontal, "Alamat");
    model->setHeaderData(3, Qt::Horizontal, "Kelurahan");
    model->setHeaderData(4, Qt::Horizontal, "Kecamatan");
    model->setHeaderData(5, Qt::Horizontal, "Kota");
    model->setHeaderData(6, Qt::Horizontal, "Propinsi");
    model->setHeaderData(7, Qt::Horizontal, "Bentuk Hukum");
    model->setHeaderData(8, Qt::Horizontal, "Jenis");
    model->setHeaderData(9, Qt::Horizontal, "KLU");
    model->setHeaderData(10, Qt::Horizontal, "Tanggal Daftar");
    model->setHeaderData(11, Qt::Horizontal, "Tanggal PKP");
    model->setHeaderData(12, Qt::Horizontal, "Tanggal PKP Cabut");
    model->setHeaderData(13, Qt::Horizontal, "NIK");
    model->setHeaderData(14, Qt::Horizontal, "Telp");
    model->setHeaderData(15, Qt::Horizontal, "Status");
    model->setHeaderData(16, Qt::Horizontal, "Nama PJ");

    mResultView->setModel(model);
    mResultView->setColumnWidth(0, 150);
    mResultView->setColumnWidth(1, 205);
    mResultView->setColumnWidth(2, 250);
    mResultView->setColumnWidth(3, 150);
    mResultView->setColumnWidth(4, 150);
    mResultView->setColumnWidth(5, 150);
    mResultView->setColumnWidth(6, 150);
    mResultView->setColumnWidth(7, 80);
    mResultView->setColumnWidth(8, 80);
    mResultView->setColumnWidth(9, 80);
    mResultView->setColumnWidth(10, 100);
    mResultView->setColumnWidth(11, 100);
    mResultView->setColumnWidth(12, 100);
    mResultView->setColumnWidth(13, 140);
    mResultView->setColumnWidth(14, 100);
    mResultView->setColumnWidth(15, 120);
    mResultView->setColumnWidth(16, 250);

    mResultView->setItemDelegateForColumn(10, new DateDelegate(this));
    mResultView->setItemDelegateForColumn(11, new DateDelegate(this));
    mResultView->setItemDelegateForColumn(12, new DateDelegate(this));

    // Context menu for copy
    mCopyAction = new QAction(QIcon(":/mfwp/src/copy.png"), "Copy", this);
    // NOTE: No setShortcut here - we handle Ctrl+C via eventFilter to
    // avoid showing "Ctrl+C" text in the context menu
    connect(mCopyAction, SIGNAL(triggered()), this, SLOT(copySelection()));

    mContextMenu = new QMenu(this);
    mContextMenu->addAction(mCopyAction);

    mResultView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mResultView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    // Handle Ctrl+C via event filter (no shortcut text in menu)
    mResultView->installEventFilter(this);
}

MasterfileSearchPage::~MasterfileSearchPage()
{
}

void MasterfileSearchPage::updateFilter()
{
    mNpwpLineEdit->setEnabled(mNpwpCheckBox->isChecked());
    mNamaLineEdit->setEnabled(mNamaCheckBox->isChecked());
    mKluLineEdit->setEnabled(mKluCheckBox->isChecked());
    mKluSelectButton->setEnabled(mKluCheckBox->isChecked());
    mAlamatLineEdit->setEnabled(mAlamatCheckBox->isChecked());
    mJenisWpComboBox->setEnabled(mJenisWpCheckBox->isChecked());
    mStatusWpComboBox->setEnabled(mStatusWpCheckBox->isChecked());
    mTglDaftarStartDateEdit->setEnabled(mTglDaftarCheckBox->isChecked());
    mTglDaftarEndDateEdit->setEnabled(mTglDaftarCheckBox->isChecked());

    Kantor kantor = mEngine->data()->kantor(mKantorComboBox->currentData().toString());
    if (kantor.type == Kanwil) {
        mSeksiCheckBox->setChecked(false);
        mSeksiCheckBox->setEnabled(false);
        mSeksiComboBox->setEnabled(false);
        mArCheckBox->setChecked(false);
        mArCheckBox->setEnabled(false);
        mPjComboBox->setEnabled(false);
    } else {
        mSeksiCheckBox->setEnabled(true);
        mSeksiComboBox->setEnabled(mSeksiCheckBox->isChecked());
        mArCheckBox->setEnabled(true);
        mPjComboBox->setEnabled(mArCheckBox->isChecked());
    }
}

void MasterfileSearchPage::updateFilterKantor()
{
    const QString kodeKantor = mKantorComboBox->currentData().toString();
    Kantor kantor = mEngine->data()->kantor(kodeKantor);

    mSeksiComboBox->clear();
    mPjComboBox->clear();
    if (kantor.type == Kanwil) return;

    QList<Seksi> seksiList = mEngine->data()->seksiList(kodeKantor);
    std::sort(seksiList.begin(), seksiList.end(), [](const Seksi &a, const Seksi &b) {
        return a.nama < b.nama;
    });
    
    foreach (const Seksi &seksi, seksiList) {
        if (seksi.type == SeksiPengawasanDanKonsultasiPelayanan || seksi.type == SeksiPengawasanDanKonsultasiPengawasan) {
            mSeksiComboBox->addItem(seksi.nama, seksi.id);
        }
    }

    QList<Pegawai> pegawaiList = mEngine->data()->pegawaiList(kodeKantor, QDate::currentDate().year());
    std::sort(pegawaiList.begin(), pegawaiList.end(), [](const Pegawai &a, const Pegawai &b) {
        return a.nama < b.nama;
    });
    
    foreach (const Pegawai &pegawai, pegawaiList) {
        int seksiTipe = pegawai.seksi.type;
        int jabatanTipe = pegawai.jabatan.type;
        // Also accept seksiTipe == 5 (SeksiPengawasanDanKonsultasiPelayanan) due to DB variations
        if (((seksiTipe == SeksiPengawasanDanKonsultasiPengawasan || seksiTipe == SeksiPengawasanDanKonsultasiPelayanan || seksiTipe == 5) && jabatanTipe == JabatanAccountRepresentativePengawasan) ||
            (seksiTipe == SeksiEkstensifikasiPerpajakan && (jabatanTipe == JabatanPelaksana || jabatanTipe == JabatanAccountRepresentativePengawasan))) {
            mPjComboBox->addItem(pegawai.nama, pegawai.nip);
        }
    }
    mPjComboBox->addItem("Unassign", "unassign");
}

void MasterfileSearchPage::clear()
{
    if (mResultView->model()) {
        static_cast<QStandardItemModel *>(mResultView->model())->removeRows(0, mResultView->model()->rowCount());
    }
}

void MasterfileSearchPage::search()
{
    QVariantMap args;
    Kantor kantor = mEngine->data()->kantor(mKantorComboBox->currentData().toString());
    args["kodeKpp"] = kantor.kode;

    if (kantor.type == Kanwil) args["isKanwil"] = true;

    if (mNpwpCheckBox->isChecked()) args["npwp"] = mNpwpLineEdit->text();
    if (mNamaCheckBox->isChecked()) args["nama"] = mNamaLineEdit->text();
    if (mKluCheckBox->isChecked()) args["klu"] = mKluLineEdit->text();
    if (mAlamatCheckBox->isChecked()) args["alamat"] = mAlamatLineEdit->text();
    if (mJenisWpCheckBox->isChecked()) args["jenis"] = mJenisWpComboBox->currentData().toString();
    if (mStatusWpCheckBox->isChecked()) args["status"] = mStatusWpComboBox->currentText();
    if (mTglDaftarCheckBox->isChecked()) {
        args["fromTanggalDaftar"] = mTglDaftarStartDateEdit->date();
        args["toTanggalDaftar"] = mTglDaftarEndDateEdit->date();
    }
    if (mSeksiCheckBox->isChecked()) args["seksiId"] = mSeksiComboBox->currentData();
    if (mArCheckBox->isChecked()) args["nipPj"] = mPjComboBox->currentData();

    clear();
    mStatusLabel->setText("Searching...");
    
    mFunction->search(args);
}

void MasterfileSearchPage::appendRows(const QList<QVariantList> &rows)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(mResultView->model());
    if (!model) return;

    for (const QVariantList &rowData : rows) {
        QList<QStandardItem *> items;
        for (int i = 0; i < 17; ++i) { // Exclude the raw npwp at the end
            QStandardItem *item = new QStandardItem();
            if (rowData[i].typeId() == QMetaType::QDate) {
                item->setData(rowData[i].toDate(), Qt::DisplayRole);
                item->setTextAlignment(Qt::AlignCenter);
            } else {
                item->setText(rowData[i].toString());
                // Align center for specific columns
                if (i == 0 || i == 3 || i == 4 || i == 5 || i == 6 || i == 7 || i == 8 || i == 9 || i >= 13) {
                    item->setTextAlignment(Qt::AlignCenter);
                }
            }
            items << item;
        }
        
        model->appendRow(items);
    }
}

void MasterfileSearchPage::searchFinished(int count)
{

    QString message;
    if (count == 0)
        message = "Data tidak ditemukan.";
    else
        message = QString("Data Wajib Pajak ditemukan %L1").arg(count);

    mStatusLabel->setText("Pencarian Selesai. " + message);
}

void MasterfileSearchPage::save()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Result", QString(), "Xlsx Files (*.xlsx)");
    if (filename.isEmpty())
        return;

    QAbstractItemModel *model = mResultView->model();
    if (!model) return;

    QXlsx::Document *xlsx = new QXlsx::Document();

    // Header
    for (int col = 0; col < model->columnCount(); ++col) {
        // QXlsx uses 1-based indexing for rows and columns
        xlsx->write(1, col + 1, model->headerData(col, Qt::Horizontal).toString());
    }

    QProgressDialog *progress = new QProgressDialog("Menyiapkan data Excel...", "Batal", 0, model->rowCount(), this);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    // Data
    for (int row = 0; row < model->rowCount(); ++row) {
        progress->setValue(row);
        QApplication::processEvents();

        if (progress->wasCanceled()) {
            progress->deleteLater();
            delete xlsx;
            return;
        }

        for (int col = 0; col < model->columnCount(); ++col) {
            QVariant value = model->index(row, col).data(Qt::DisplayRole);
            if (value.typeId() == QMetaType::QDate) {
                xlsx->write(row + 2, col + 1, value.toDate().toString("dd-MM-yyyy"));
            } else {
                xlsx->write(row + 2, col + 1, value.toString());
            }
        }
    }
    
    progress->setLabelText("Menulis file ke disk... Mohon tunggu.");
    progress->setCancelButton(nullptr); // Disable cancel during disk write
    progress->setRange(0, 0); // Show indeterminate progress
    QApplication::processEvents();

    QThread *thread = QThread::create([=]() {
        bool success = xlsx->saveAs(filename);
        delete xlsx;
        
        QMetaObject::invokeMethod(this, [=]() {
            progress->close();
            if (!success) {
                QMessageBox::warning(this, "Save Error", "Gagal menyimpan file.");
            } else {
                QMessageBox::information(this, "Sukses", "Data berhasil disimpan.");
            }
            progress->deleteLater();
        }, Qt::QueuedConnection);
    });
    
    thread->start();
}

void MasterfileSearchPage::selectKlu()
{
    KluSelectDialog dialog(mEngine, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString klu = dialog.selectedKlu();
        if (!klu.isEmpty()) {
            QString existing = mKluLineEdit->text();
            if (existing.isEmpty())
                mKluLineEdit->setText(klu);
            else
                mKluLineEdit->setText(existing + "," + klu);
        }
    }
}

bool MasterfileSearchPage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == mResultView && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Copy)) {
            copySelection();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MasterfileSearchPage::copySelection()
{
    QModelIndexList indexList = mResultView->selectionModel()->selectedIndexes();
    if (indexList.isEmpty()) return;

    int minRow = indexList.first().row();
    int minCol = indexList.first().column();
    int maxRow = minRow;
    int maxCol = minCol;

    foreach (const QModelIndex &idx, indexList) {
        if (idx.row() < minRow) minRow = idx.row();
        if (idx.row() > maxRow) maxRow = idx.row();
        if (idx.column() < minCol) minCol = idx.column();
        if (idx.column() > maxCol) maxCol = idx.column();
    }

    QHash<QPair<int,int>, QString> cellData;
    foreach (const QModelIndex &idx, indexList) {
        QVariant val = idx.data(Qt::DisplayRole);
        QString text;
        if (val.typeId() == QMetaType::QDate)
            text = val.toDate().toString("dd-MM-yyyy");
        else
            text = val.toString();
        cellData[{idx.row(), idx.column()}] = text;
    }

    QString plainText;
    QString htmlText = "<table><tbody>";

    for (int row = minRow; row <= maxRow; ++row) {
        htmlText += "<tr>";
        QStringList rowCells;
        for (int col = minCol; col <= maxCol; ++col) {
            QString val = cellData.value({row, col});
            rowCells << val;
            htmlText += QString("<td>%1</td>").arg(val.toHtmlEscaped());
        }
        plainText += rowCells.join("\t") + "\n";
        htmlText += "</tr>";
    }
    htmlText += "</tbody></table>";

    QMimeData *mimeData = new QMimeData;
    mimeData->setHtml(htmlText);
    mimeData->setText(plainText.trimmed());
    QApplication::clipboard()->setMimeData(mimeData);
}

void MasterfileSearchPage::showContextMenu(const QPoint &pos)
{
    if (!mResultView->selectionModel()->hasSelection()) return;
    mContextMenu->popup(mResultView->viewport()->mapToGlobal(pos));
}
