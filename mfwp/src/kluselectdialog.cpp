#include "kluselectdialog.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QFont>
#include <QSortFilterProxyModel>

#include <coreengine.h>
#include <database.h>
#include <common.h>

KluSelectDialog::KluSelectDialog(CoreEngine *engine, QWidget *parent)
    : QDialog(parent), mEngine(engine)
{
    mResultView = new QTableView;
    mResultView->setAlternatingRowColors(true);
    mResultView->setWordWrap(false);
    mResultView->setSortingEnabled(true);
    mResultView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mResultView->setSelectionMode(QAbstractItemView::SingleSelection);
    mResultView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QFont tableFont = mResultView->font();
    tableFont.setPointSize(8);
    mResultView->setFont(tableFont);

    mResultView->horizontalHeader()->setStretchLastSection(false);
    mResultView->horizontalHeader()->setHighlightSections(false);
    mResultView->verticalHeader()->hide();
    mResultView->verticalHeader()->setDefaultSectionSize(mResultView->fontMetrics().height() + 10);
    mResultView->verticalHeader()->setHighlightSections(false);

    mKeywordEdit = new QLineEdit;

    QPushButton *pilihButton = new QPushButton("Pilih");
    QPushButton *batalButton = new QPushButton("Batal");
    pilihButton->setFixedWidth(60);
    batalButton->setFixedWidth(60);

    QVBoxLayout *btnLayout = new QVBoxLayout;
    btnLayout->addWidget(pilihButton);
    btnLayout->addWidget(batalButton);
    btnLayout->addStretch();

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("Search"));
    searchLayout->addWidget(mKeywordEdit);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(mResultView);
    contentLayout->addLayout(btnLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(searchLayout);
    layout->addLayout(contentLayout);

    setLayout(layout);
    setWindowTitle("Referensi - KLU");
    resize(700, 450);

    connect(mKeywordEdit, SIGNAL(textChanged(QString)), SLOT(search()));
    connect(pilihButton, SIGNAL(clicked()), SLOT(accept()));
    connect(batalButton, SIGNAL(clicked()), SLOT(reject()));
    connect(mResultView, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClicked()));

    search();
}

void KluSelectDialog::search()
{
    delete mResultView->model();

    QStandardItemModel *model = new QStandardItemModel(0, 3, mResultView);
    model->setHeaderData(0, Qt::Horizontal, "KLU");
    model->setHeaderData(1, Qt::Horizontal, "Uraian");
    model->setHeaderData(2, Qt::Horizontal, "Sektor");

    DatabasePtr db = mEngine->database();
    QString keyword = mKeywordEdit->text();
    QString searchSql = QString("SELECT `kode`, `nama`, `sektor` FROM `klu` WHERE `kode` LIKE '%1%' OR `nama` LIKE '%%1%'").arg(keyword);

    db->exec(searchSql);
    while (db->next()) {
        QString kode  = db->value(0).toString();
        QString uraian = db->value(1).toString();
        QString sektor = db->value(2).toString();

        QStandardItem *kluItem = new QStandardItem(kode);
        kluItem->setTextAlignment(Qt::AlignCenter);

        QStandardItem *uraianItem = new QStandardItem(uraian);
        QStandardItem *sektorItem = new QStandardItem(mEngine->common()->namaSektor(sektor));

        model->appendRow({ kluItem, uraianItem, sektorItem });
    }

    mResultView->setModel(model);
    mResultView->setColumnWidth(0, 80);
    mResultView->setColumnWidth(1, 400);
    mResultView->setColumnWidth(2, 150);
}

void KluSelectDialog::accept()
{
    QModelIndexList selected = mResultView->selectionModel()->selectedRows(0);
    if (!selected.isEmpty())
        mSelectedKlu = selected.first().data().toString();
    QDialog::accept();
}

void KluSelectDialog::onDoubleClicked()
{
    accept();
}
