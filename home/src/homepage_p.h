#ifndef HOMEPAGE_P_H
#define HOMEPAGE_P_H

#include <QObject>
#include <QMetaObject>
#include <QList>
#include <QVariantMap>

#include <coreengine.h>
#include <runnable.h>
#include <enums.h>

class HomeFunction;

// ---------------------------------------------------------------------------
// HomeDataRunnable – runs all DB queries synchronously in a background thread
// Results are dispatched back to the main thread via invokeMethod (Queued).
// ---------------------------------------------------------------------------
class HomeDataRunnable : public Runnable
{
public:
    HomeDataRunnable(CoreEngine *engine, HomeFunction *fn,
                     const QString &kodeKantor, const QStringList &kantorList,
                     int tahun)
        : mEngine(engine), mFunction(fn), mKantor(kodeKantor),
          mKantorList(kantorList), mTahun(tahun) {}

    void run() override;

private:
    CoreEngine   *mEngine;
    HomeFunction *mFunction;
    QString       mKantor;
    QStringList   mKantorList;
    int           mTahun;
};

// ---------------------------------------------------------------------------
// HomeFunction – lightweight QObject that owns the async job and emits signals
// ---------------------------------------------------------------------------
class HomeFunction : public QObject
{
    Q_OBJECT

    CoreEngine  *mEngine;
    QString      mKantor;
    QStringList  mKantorList;
    int          mTahun;

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
        DataPbkCurrentYear,
        DataLastMpnDate,
        DataLastSpmDate
    };

    HomeFunction(CoreEngine *engine, QObject *parent = nullptr)
        : QObject(parent), mEngine(engine), mTahun(0) {}

    void setKantor(const QString &kodeKantor) { mKantor = kodeKantor; }
    void setKantorList(const QStringList &list) { mKantorList = list; }
    void setTahun(int tahun) { mTahun = tahun; }

    void start()
    {
        mEngine->runAsync(
            new HomeDataRunnable(mEngine, this, mKantor, mKantorList, mTahun));
    }

public slots:
    // Called by HomeDataRunnable via QMetaObject::invokeMethod (QueuedConnection)
    void emitGotData(const QVariantMap &data) { emit gotData(data); }
    void emitFinished()                        { emit finished(); }

signals:
    void finished();
    void gotData(const QVariantMap &data);
};

// ---------------------------------------------------------------------------
// HomeDataRunnable::run  – executes on a worker thread
// ---------------------------------------------------------------------------
inline void HomeDataRunnable::run()
{
    DatabasePtr db = mEngine->database();
    if (!db)  {
        QMetaObject::invokeMethod(mFunction, "emitFinished", Qt::QueuedConnection);
        return;
    }

    // Build filter strings.
    // MPN table uses column `admin` for KPP code.
    // SPMKP / SPMPP / Renpen use column `kpp`.
    // PBK uses `kpp` and `kpp2`.
    QString filterMpn, filterSpmkp, filterSpmpp, filterRenpen, filterPbk;

    if (!mKantorList.isEmpty()) {
        // Build an IN list for each type
        QStringList quoted;
        for (const QString &k : mKantorList)
            quoted << QString("'%1'").arg(k);
        QString inList = quoted.join(",");

        filterMpn     = QString("`admin` IN (%1)").arg(inList);
        filterSpmkp   = QString("`kpp` IN (%1)").arg(inList);
        filterSpmpp   = QString("`kpp` IN (%1)").arg(inList);
        filterRenpen  = QString("`kpp` IN (%1)").arg(inList);
        filterPbk     = QString("(`kpp` IN (%1) OR `kpp2` IN (%1))").arg(inList);
    } else {
        // Single kantor
        filterMpn     = QString("`admin` = '%1'").arg(mKantor);
        filterSpmkp   = QString("`kpp` = '%1'").arg(mKantor);
        filterSpmpp   = QString("`kpp` = '%1'").arg(mKantor);
        filterRenpen  = QString("`kpp` = '%1'").arg(mKantor);
        filterPbk     = QString("(`kpp`='%1' OR `kpp2`='%1')").arg(mKantor);
    }

    // -----------------------------------------------------------------------
    // 1) MPN (penerimaan) – current & previous year
    // -----------------------------------------------------------------------
    for (int i = 0; i <= 1; i++) {
        int yearQuery = mTahun - i;
        QString sql = QString(
            "SELECT `npwp`, `kpp`, `cabang`, `bulanbayar`, SUM(`nominal`), `source`, `tahunbayar` "
            "FROM `mpn` WHERE `tahunbayar`='%1' AND %2 "
            "GROUP BY `npwp`, `kpp`, `cabang`, `bulanbayar`, `source` ORDER BY `bulanbayar`"
        ).arg(yearQuery).arg(filterMpn);

        db->exec(sql);
        while (db->next()) {
            int  source = db->value(5).toInt();
            int  tahun  = db->value(6).toInt();

            int dataType;
            if (source == MpnSourceMpn)
                dataType = (tahun == mTahun) ? HomeFunction::DataMpnCurrentYear : HomeFunction::DataMpnLastYear;
            else if (source == MpnSourceSpm)
                dataType = (tahun == mTahun) ? HomeFunction::DataSpmCurrentYear : HomeFunction::DataSpmLastYear;
            else
                continue;

            QVariantMap data;
            data["type"]   = dataType;
            data["npwp"]   = db->value(0).toString();
            data["kpp"]    = db->value(1).toString();
            data["cabang"] = db->value(2).toString();
            data["bulan"]  = db->value(3).toInt();
            data["nominal"]= db->value(4).toDouble();

            QMetaObject::invokeMethod(mFunction, "emitGotData",
                Qt::QueuedConnection, Q_ARG(QVariantMap, data));
        }
    }

    // -----------------------------------------------------------------------
    // 2) SPMKP – current & previous year
    // -----------------------------------------------------------------------
    for (int i = 0; i <= 1; i++) {
        int yearQuery = mTahun - i;
        QString sql = QString(
            "SELECT `npwp`, `kpp`, `cabang`, `bulan`, SUM(`nominal`), `tahun` "
            "FROM `spmkp` WHERE `tahun`='%1' AND %2 "
            "GROUP BY `npwp`, `kpp`, `cabang`, `bulan` ORDER BY `bulan`"
        ).arg(yearQuery).arg(filterSpmkp);

        db->exec(sql);
        while (db->next()) {
            int tahun = db->value(5).toInt();
            QVariantMap data;
            data["type"]   = (tahun == mTahun) ? HomeFunction::DataSpmkpCurrentYear : HomeFunction::DataSpmkpLastYear;
            data["npwp"]   = db->value(0).toString();
            data["kpp"]    = db->value(1).toString();
            data["cabang"] = db->value(2).toString();
            data["bulan"]  = db->value(3).toInt();
            data["nominal"]= db->value(4).toDouble();
            QMetaObject::invokeMethod(mFunction, "emitGotData",
                Qt::QueuedConnection, Q_ARG(QVariantMap, data));
        }
    }

    // -----------------------------------------------------------------------
    // 3) SPMPP – current & previous year
    // -----------------------------------------------------------------------
    for (int i = 0; i <= 1; i++) {
        int yearQuery = mTahun - i;
        QString sql = QString(
            "SELECT `npwp`, `kpp`, `cabang`, `bulan`, SUM(`nominal`), `tahun` "
            "FROM `spmpp` WHERE `tahun`='%1' AND %2 "
            "GROUP BY `npwp`, `kpp`, `cabang`, `bulan` ORDER BY `bulan`"
        ).arg(yearQuery).arg(filterSpmpp);

        db->exec(sql);
        while (db->next()) {
            int tahun = db->value(5).toInt();
            QVariantMap data;
            data["type"]   = (tahun == mTahun) ? HomeFunction::DataSpmppCurrentYear : HomeFunction::DataSpmppLastYear;
            data["npwp"]   = db->value(0).toString();
            data["kpp"]    = db->value(1).toString();
            data["cabang"] = db->value(2).toString();
            data["bulan"]  = db->value(3).toInt();
            data["nominal"]= db->value(4).toDouble();
            QMetaObject::invokeMethod(mFunction, "emitGotData",
                Qt::QueuedConnection, Q_ARG(QVariantMap, data));
        }
    }

    // -----------------------------------------------------------------------
    // 4) PBK – current & previous year
    // -----------------------------------------------------------------------
    for (int i = 0; i <= 1; i++) {
        int yearQuery = mTahun - i;
        QString sql;
        if (db->driverName() == "QMYSQL") {
            sql = QString(
                "SELECT YEAR(`tanggaldoc`) as `tahundoc`, MONTH(`tanggaldoc`) as `bulandoc`, "
                "`npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`, SUM(`nominal`) "
                "FROM `pbk` WHERE YEAR(`tanggaldoc`)='%1' AND %2 "
                "GROUP BY `tahundoc`, `bulandoc`, `npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`"
            ).arg(yearQuery).arg(filterPbk);
        } else {
            sql = QString(
                "SELECT strftime('%%Y',`tanggaldoc`) as `tahundoc`, strftime('%%m',`tanggaldoc`) as `bulandoc`, "
                "`npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`, SUM(`nominal`) "
                "FROM `pbk` WHERE strftime('%%Y',`tanggaldoc`)='%1' AND %2 "
                "GROUP BY `tahundoc`, `bulandoc`, `npwp`, `kpp`, `cabang`, `npwp2`, `kpp2`, `cabang2`"
            ).arg(yearQuery).arg(filterPbk);
        }

        db->exec(sql);
        while (db->next()) {
            int tahun = db->value(0).toInt();
            QVariantMap data;
            data["type"]   = (tahun == mTahun) ? HomeFunction::DataPbkCurrentYear : HomeFunction::DataPbkLastYear;
            data["bulan"]  = db->value(1).toInt();
            data["npwp"]   = db->value(2).toString();
            data["kpp"]    = db->value(3).toString();
            data["cabang"] = db->value(4).toString();
            data["npwp2"]  = db->value(5).toString();
            data["kpp2"]   = db->value(6).toString();
            data["cabang2"]= db->value(7).toString();
            data["nominal"]= db->value(8).toDouble();
            QMetaObject::invokeMethod(mFunction, "emitGotData",
                Qt::QueuedConnection, Q_ARG(QVariantMap, data));
        }
    }

    // -----------------------------------------------------------------------
    // 5) Renpen (rencana penerimaan) – current year only
    // -----------------------------------------------------------------------
    {
        QString sql = QString(
            "SELECT `nip`, `bulan`, SUM(`target`) "
            "FROM `renpen` WHERE `tahun`='%1' AND %2 "
            "GROUP BY `nip`, `bulan`"
        ).arg(mTahun).arg(filterRenpen);

        db->exec(sql);
        while (db->next()) {
            QVariantMap data;
            data["type"]   = HomeFunction::DataRenpenCurrentYear;
            data["nip"]    = db->value(0).toString();
            data["bulan"]  = db->value(1).toInt();
            data["nominal"]= db->value(2).toDouble();
            QMetaObject::invokeMethod(mFunction, "emitGotData",
                Qt::QueuedConnection, Q_ARG(QVariantMap, data));
        }
    }

    // -----------------------------------------------------------------------
    // 6) Last MPN date: MAX(datebayar) from mpn WHERE source=1
    // -----------------------------------------------------------------------
    {
        QString sql = QString(
            "SELECT MAX(`datebayar`) FROM `mpn` WHERE `source`=%1 AND %2"
        ).arg(MpnSourceMpn).arg(filterMpn);
        db->exec(sql);
        if (db->next()) {
            QDate lastDate = db->value(0).toDate();
            if (lastDate.isValid()) {
                QVariantMap data;
                data["type"]  = HomeFunction::DataLastMpnDate;
                data["value"] = lastDate.toString("dd-MM-yyyy");
                QMetaObject::invokeMethod(mFunction, "emitGotData",
                    Qt::QueuedConnection, Q_ARG(QVariantMap, data));
            }
        }
    }

    // -----------------------------------------------------------------------
    // 7) Last SPM date: MAX(datebayar) from mpn WHERE source=2
    // -----------------------------------------------------------------------
    {
        QString sql = QString(
            "SELECT MAX(`datebayar`) FROM `mpn` WHERE `source`=%1 AND %2"
        ).arg(MpnSourceSpm).arg(filterMpn);
        db->exec(sql);
        if (db->next()) {
            QDate lastDate = db->value(0).toDate();
            if (lastDate.isValid()) {
                QVariantMap data;
                data["type"]  = HomeFunction::DataLastSpmDate;
                data["value"] = lastDate.toString("dd-MM-yyyy");
                QMetaObject::invokeMethod(mFunction, "emitGotData",
                    Qt::QueuedConnection, Q_ARG(QVariantMap, data));
            }
        }
    }

    // Signal completion back to main thread
    QMetaObject::invokeMethod(mFunction, "emitFinished", Qt::QueuedConnection);
}

#endif // HOMEPAGE_P_H
