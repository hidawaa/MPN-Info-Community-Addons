#include "masterfilesearch_p.h"

#include <QDate>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QStringList>
#include <QRegularExpression>

#include <common.h>
#include <coredata.h>

MasterfileSearchFunction::MasterfileSearchFunction(CoreEngine *engine, QObject *parent)
    : QObject(parent), mEngine(engine)
{
}

void MasterfileSearchFunction::search(const QVariantMap &args)
{
    MasterfileSearchRunnable *runnable = new MasterfileSearchRunnable(mEngine, this, args);
    mEngine->runAsync(runnable);
}

MasterfileSearchRunnable::MasterfileSearchRunnable(CoreEngine *engine, MasterfileSearchFunction *fn, const QVariantMap &args)
    : mEngine(engine), mFunction(fn), mArgs(args)
{
}

void MasterfileSearchRunnable::run()
{
    QString npwpFull = mArgs["npwp"].toString();
    QString nama = mArgs["nama"].toString();
    QString kluArg = mArgs["klu"].toString();
    QString alamat = mArgs["alamat"].toString();
    QString jenis = mArgs["jenis"].toString();
    QString status = mArgs["status"].toString();
    QString kodeKpp = mArgs["kodeKpp"].toString();
    bool isKanwil = mArgs["isKanwil"].toBool();
    QDate fromTanggalDaftar = mArgs["fromTanggalDaftar"].isValid() ? mArgs["fromTanggalDaftar"].toDate() : QDate();
    QDate toTanggalDaftar = mArgs["toTanggalDaftar"].isValid() ? mArgs["toTanggalDaftar"].toDate() : QDate();
    QString nipPj = mArgs["nipPj"].toString();
    int seksiId = mArgs.contains("seksiId") ? mArgs["seksiId"].toInt() : -1;

    QString npwpFilter, kppFilter, cabangFilter;
    if (!npwpFull.isEmpty()) {
        npwpFull = npwpFull.remove(QRegularExpression("[^0-9]"));
        if (npwpFull.length() >= 9) npwpFilter = npwpFull.mid(0, 9);
        if (npwpFull.length() >= 12) kppFilter = npwpFull.mid(9, 3);
        if (npwpFull.length() == 15) cabangFilter = npwpFull.mid(12, 3);
    }
    
    QStringList kluList = kluArg.split(",", Qt::SkipEmptyParts);

    QStringList filterKantorList;
    if (!kodeKpp.isEmpty() && isKanwil) {
        KantorList kantorList = mEngine->data()->kppList(kodeKpp);
        foreach (const Kantor &kantor, kantorList)
            filterKantorList << kantor.kode;
    }

    PegawaiList pegawaiList = mEngine->data()->pegawaiList(kodeKpp, QDate::currentDate().year());
    QHash<QString, Pegawai> pegawaiHash;
    foreach (const Pegawai &p, pegawaiList) {
        pegawaiHash[p.nip] = p;
    }

    QList<QVariantList> allRows;

    int count = 0;
    const WajibPajakHash &hash = mEngine->data()->wajibPajakHash();
    
    for (auto it = hash.constBegin(); it != hash.constEnd(); ++it) {
        const WajibPajak &wp = it.value();

        if (!npwpFilter.isEmpty() && !wp.npwp.contains(npwpFilter)) continue;
        if (!kppFilter.isEmpty() && wp.kpp != kppFilter) continue;
        if (!cabangFilter.isEmpty() && wp.cabang != cabangFilter) continue;
        
        if (!nama.isEmpty() && !wp.nama.contains(nama, Qt::CaseInsensitive)) continue;
        
        if (!kluList.isEmpty()) {
            bool matchLike = false;
            bool matchNotLike = true;
            int likeCount = 0;
            for (const QString &k : kluList) {
                QString kluTarget = k;
                if (kluTarget.startsWith("!")) {
                    kluTarget.remove(0, 1);
                    if (wp.klu.contains(kluTarget, Qt::CaseInsensitive)) {
                        matchNotLike = false;
                        break;
                    }
                } else {
                    likeCount++;
                    if (wp.klu.contains(kluTarget, Qt::CaseInsensitive)) {
                        matchLike = true;
                    }
                }
            }
            if (!matchNotLike) continue;
            if (likeCount > 0 && !matchLike) continue;
        }

        if (!alamat.isEmpty()) {
            bool alamatMatch = wp.alamat.contains(alamat, Qt::CaseInsensitive) || 
                               wp.kelurahan.contains(alamat, Qt::CaseInsensitive) ||
                               wp.kecamatan.contains(alamat, Qt::CaseInsensitive) ||
                               wp.kota.contains(alamat, Qt::CaseInsensitive);
            if (!alamatMatch) continue;
        }

        if (!jenis.isEmpty() && wp.jenis != jenis) continue;
        if (!status.isEmpty() && wp.status != status) continue;

        if (!kodeKpp.isEmpty()) {
            if (isKanwil) {
                if (!filterKantorList.contains(wp.admin)) continue;
            } else {
                if (wp.admin != kodeKpp) continue;
            }
        }

        if (!fromTanggalDaftar.isNull() && !toTanggalDaftar.isNull()) {
            if (wp.tgl_daftar < fromTanggalDaftar || wp.tgl_daftar > toTanggalDaftar) continue;
        }

        if (!nipPj.isEmpty()) {
            if (nipPj == "unassign") {
                if (!wp.ar.isEmpty() || !wp.eks.isEmpty() || !wp.js.isEmpty()) continue;
            } else {
                if (wp.ar != nipPj && wp.eks != nipPj && wp.js != nipPj) continue;
            }
        }

        if (seksiId != -1) {
            QString nip = !wp.ar.isEmpty() ? wp.ar : (!wp.eks.isEmpty() ? wp.eks : wp.js);
            if (nip.isEmpty()) continue;
            
            auto itPegawai = pegawaiHash.constFind(nip);
            if (itPegawai == pegawaiHash.constEnd() || itPegawai.value().seksi.id != seksiId) continue;
        }

        QVariantList row;
        row << mEngine->common()->formattedNpwp(wp.npwp, wp.kpp, wp.cabang);
        row << wp.nama;
        row << wp.alamat;
        row << wp.kelurahan;
        row << wp.kecamatan;
        row << wp.kota;
        row << wp.propinsi;
        row << wp.bhkm;
        row << wp.jenis;
        row << wp.klu;
        row << wp.tgl_daftar;
        row << wp.tgl_pkp;
        row << wp.tgl_pkp_cabut;
        row << wp.nik;
        row << wp.telp;
        row << wp.status;

        QString nip = !wp.ar.isEmpty() ? wp.ar : (!wp.eks.isEmpty() ? wp.eks : wp.js);
        QString namaPegawai = pegawaiHash.value(nip).nama;
        if (nip.isEmpty()) {
             row << QString("");
        } else {
             row << QString("%1 - %2").arg(nip, namaPegawai);
        }

        row << wp.npwp;

        allRows.append(row);
        count++;
    }

    QMetaObject::invokeMethod(mFunction, "onRowsReady", Qt::QueuedConnection, Q_ARG(QList<QVariantList>, allRows));
    QMetaObject::invokeMethod(mFunction, "onFinished", Qt::QueuedConnection, Q_ARG(int, count));
}
