#ifndef HOMEPAGE_P_H
#define HOMEPAGE_P_H

#include <QObject>
#include <QSqlQuery>

#include <coreengine.h>

class HomeFunction : public QObject
{
    Q_OBJECT

    CoreEngine *mEngine;
    QString mKantor;
    int mTahun;

public:
    enum DataType {
        DataMpnCurrentYear = 0,
        DataSpmCurrentYear,
        DataMpnLastYear,
        DataSpmLastYear,
        DataSpmkpCurrentYear,
        DataSpmkpLastYear,
        DataSpmppCurrentYear,
        DataSpmppLastYear,
        DataRenpenCurrentYear,
        DataPbkLastYear,
        DataPbkCurrentYear
    };

    HomeFunction(CoreEngine *engine, QObject *parent = nullptr) : QObject(parent), mEngine(engine), mNumFuture(0), mStarted(false) {}

    void setKantor(const QString &kodeKantor) { mKantor = kodeKantor; }
    void setTahun(int tahun) { mTahun = tahun; }

    void start()
    {
        Kantor kantor = mEngine->kantor(mKantor);

        /*
        QString filter, filterRenpen, filterPbk;
        if (!kantor.kpp.isEmpty())
        {
            filter += QString("`admin` = '%1'")
                    .arg(kantor.kpp);
            filterRenpen += QString("`kpp` = '%1'")
                    .arg(kantor.kpp);
            filterPbk += QString("(`kpp`='%1' OR `kpp2`='%1')")
                    .arg(kantor.kpp);
        }

        if (!filter.isEmpty())
            filter.prepend("AND ");

        if (!filterRenpen.isEmpty())
            filterRenpen.prepend("AND ");

        if (!filterPbk.isEmpty())
            filterPbk.prepend("AND ");

        QSqlQuery query(mEngine->database());

        for (int i=0; i<=1; i++)
        {
            int yearQuery = mTahun - i;
            QString totalPenerimaanBulanSql = QString("SELECT `npwp`, `kpp`, `cabang`, `bulanbayar`, SUM(`nominal`), `source`, `tahunbayar` FROM `mpn` WHERE `tahunbayar`='%1' %2 GROUP BY `npwp`, `kpp`, `cabang`, `bulanbayar`, `source` ORDER BY `bulanbayar`").arg(yearQuery).arg(filter);
            QueryWatcher *watcher = query.exec(totalPenerimaanBulanSql);

            connect(watcher, SIGNAL(finished()), SLOT(onPenerimaanQueryFinished()));
            mNumFuture++;
        }

        for (int i=0; i<=1; i++)
        {
            int yearQuery = mTahun - i;
            QString totalSpmkpSql = QString("SELECT `npwp`, `kpp`, `cabang`, `bulan`, SUM(`nominal`), `tahun` FROM `spmkp` WHERE `tahun`='%1' %2 GROUP BY `npwp`, `kpp`, `cabang`, `bulan` ORDER BY `bulan`").arg(yearQuery).arg(filter);
            QueryWatcher *watcher = query.exec(totalSpmkpSql);

            connect(watcher, SIGNAL(finished()), SLOT(onSpmkpQueryFinished()));
            mNumFuture++;
        }

        for (int i=0; i<=1; i++) {
            int yearQuery = mTahun - i;
            QString totalSpmppSql = QString("SELECT `npwp`, `kpp`, `cabang`, `bulan`, SUM(`nominal`), `tahun` FROM `spmpp` WHERE `tahun`='%1' %2 GROUP BY `npwp`, `kpp`, `cabang`, `bulan` ORDER BY `bulan`").arg(yearQuery).arg(filter);
            QueryWatcher *watcher = query.exec(totalSpmppSql);

            connect(watcher, SIGNAL(finished()), SLOT(onSpmppQueryFinished()));
            mNumFuture++;
        }

        for (int i=0; i<=1; i++) {
            int yearQuery = mTahun - i;

            QString totalPbkSql;
            if (mEngine->databaseDriverName() == IDD_DATABASE_DRIVER_MYSQL)
                totalPbkSql = QString("SELECT YEAR(`tanggaldoc`) as `tahundoc`, MONTH(`tanggaldoc`) as `bulandoc`, `npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`, SUM(`nominal`) FROM `pbk` WHERE YEAR(`tanggaldoc`)='%1' %2 GROUP BY `tahundoc`, `bulandoc`, `npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`").arg(yearQuery).arg(filterPbk);
            else
                totalPbkSql = QString("SELECT strftime('%Y',`tanggaldoc`) as `tahundoc`, strftime('%m',`tanggaldoc`) as `bulandoc`, `npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`, SUM(`nominal`) FROM `pbk` WHERE strftime('%Y',`tanggaldoc`)='%1' %2 GROUP BY `tahundoc`, `bulandoc`, `npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`").arg(yearQuery).arg(filterPbk);
            QueryWatcher *watcher = query.exec(totalPbkSql);

            connect(watcher, SIGNAL(finished()), SLOT(onPbkQueryFinished()));
            mNumFuture++;
        }

        {
            QString totalRenpenSql = QString("SELECT `nip`, `bulan`, SUM(`target`) FROM `renpen` WHERE `tahun`='%1' %2 GROUP BY `nip`, `bulan`").arg(mTahun).arg(filterRenpen);
            QueryWatcher *watcher = query.exec(totalRenpenSql);

            connect(watcher, SIGNAL(finished()), SLOT(onRenpenQueryFinished()));
            mNumFuture++;
        }

        mStarted = true;
        */
    }

signals:
    void finished();
    void gotData(const QVariantMap &data);

private slots:
    void onPenerimaanQueryFinished() {
        /*
        QueryWatcher *watcher = static_cast<QueryWatcher *>(QObject::sender());

        RecordList resultList = watcher->result();
        watcher->deleteLater();

        foreach (const QSqlRecord &record, resultList)
        {
            QString npwp = record.value(0).toString();
            QString kpp = record.value(1).toString();
            QString cabang = record.value(2).toString();
            int bulan = record.value(3).toInt();
            double nominal = record.value(4).toDouble();
            int source = record.value(5).toInt();
            int tahun = record.value(6).toInt();

            QVariantMap data;
            data["npwp"] = npwp;
            data["kpp"] = kpp;
            data["cabang"] = cabang;
            data["bulan"] = bulan;
            data["nominal"] = nominal;

            if (source == SourceMpn) {
                if (tahun == mTahun)
                    data["type"] = DataMpnCurrentYear;
                else
                    data["type"] = DataMpnLastYear;
            }

            if (source == SourceSpm) {
                if (tahun == mTahun)
                    data["type"] = DataSpmCurrentYear;
                else
                    data["type"] = DataSpmLastYear;
            }

            emit gotData(data);
        }

        mNumFuture--;
        if (mStarted && mNumFuture <= 0)
            emit finished();
        */
    }

    void onSpmkpQueryFinished() {
        /*
        QueryWatcher *watcher = static_cast<QueryWatcher *>(QObject::sender());

        RecordList resultList = watcher->result();
        watcher->deleteLater();

        foreach (const QSqlRecord &record, resultList)
        {
            QString npwp = record.value(0).toString();
            QString kpp = record.value(1).toString();
            QString cabang = record.value(2).toString();
            int bulan = record.value(3).toInt();
            double nominal = record.value(4).toDouble();
            int tahun = record.value(5).toInt();

            QVariantMap data;
            data["npwp"] = npwp;
            data["kpp"] = kpp;
            data["cabang"] = cabang;
            data["bulan"] = bulan;
            data["nominal"] = nominal;

            if (tahun == mTahun)
                data["type"] = DataSpmkpCurrentYear;
            else
                data["type"] = DataSpmkpLastYear;

            emit gotData(data);
        }

        mNumFuture--;
        if (mStarted && mNumFuture <= 0)
            emit finished();
        */
    }

    void onSpmppQueryFinished() {
        /*
        QueryWatcher *watcher = static_cast<QueryWatcher *>(QObject::sender());

        RecordList resultList = watcher->result();
        watcher->deleteLater();

        foreach (const QSqlRecord &record, resultList)
        {
            QString npwp = record.value(0).toString();
            QString kpp = record.value(1).toString();
            QString cabang = record.value(2).toString();
            int bulan = record.value(3).toInt();
            double nominal = record.value(4).toDouble();
            int tahun = record.value(5).toInt();

            QVariantMap data;
            data["npwp"] = npwp;
            data["kpp"] = kpp;
            data["cabang"] = cabang;
            data["bulan"] = bulan;
            data["nominal"] = nominal;

            if (tahun == mTahun)
                data["type"] = DataSpmppCurrentYear;
            else
                data["type"] = DataSpmppLastYear;

            emit gotData(data);
        }

        mNumFuture--;
        if (mStarted && mNumFuture <= 0)
            emit finished();
        */
    }

    void onPbkQueryFinished() {
        /*
        QueryWatcher *watcher = static_cast<QueryWatcher *>(QObject::sender());

        RecordList resultList = watcher->result();
        watcher->deleteLater();

        foreach (const QSqlRecord &record, resultList)
        {
            int tahun = record.value(0).toInt();
            int bulan = record.value(1).toInt();
            QString npwp = record.value(2).toString();
            QString kpp = record.value(3).toString();
            QString cabang = record.value(4).toString();
            QString npwp2 = record.value(5).toString();
            QString kpp2 = record.value(6).toString();
            QString cabang2 = record.value(7).toString();
            double nominal = record.value(8).toDouble();

            QVariantMap data;
            data["bulan"] = bulan;
            data["npwp"] = npwp;
            data["kpp"] = kpp;
            data["cabang"] = cabang;
            data["npwp2"] = npwp2;
            data["kpp2"] = kpp2;
            data["cabang2"] = cabang2;
            data["nominal"] = nominal;

            if (tahun == mTahun)
                data["type"] = DataPbkCurrentYear;
            else
                data["type"] = DataPbkLastYear;

            emit gotData(data);
        }

        mNumFuture--;
        if (mStarted && mNumFuture <= 0)
            emit finished();
        */
    }

    void onRenpenQueryFinished() {
        /*
        QueryWatcher *watcher = static_cast<QueryWatcher *>(QObject::sender());

        RecordList resultList = watcher->result();
        watcher->deleteLater();

        foreach (const QSqlRecord &record, resultList)
        {
            QVariantMap data;
            data["type"] = DataRenpenCurrentYear;
            data["nip"] = record.value(0).toString();;
            data["bulan"] = record.value(1).toInt();;
            data["nominal"] = record.value(2).toDouble();

            emit gotData(data);
        }

        mNumFuture--;
        if (mStarted && mNumFuture <= 0)
            emit finished();
        */
    }

private:
    int mNumFuture;
    bool mStarted;
};

#endif // HOMEPAGE_P_H
