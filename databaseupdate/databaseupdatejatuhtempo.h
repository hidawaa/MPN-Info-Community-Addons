#ifndef DATABASEUPDATEJATUHTEMPO_H
#define DATABASEUPDATEJATUHTEMPO_H

#include <QFile>

#include <coreengine.h>
#include <runnable.h>

#define IDD_JATUHTEMPO_HEADER_BULAN                             "BULAN"
#define IDD_JATUHTEMPO_HEADER_TAHUN                             "TAHUN"
#define IDD_JATUHTEMPO_HEADER_POTPUT                            "POTPUT"
#define IDD_JATUHTEMPO_HEADER_PPH                               "PPH"
#define IDD_JATUHTEMPO_HEADER_PPN                               "PPN"
#define IDD_JATUHTEMPO_HEADER_PPH_OP                            "PPHOP"
#define IDD_JATUHTEMPO_HEADER_PPH_BADAN                         "PPHBDN"

#define IDD_DATA_JATUHTEMPO_KEY        "data.jatuhtempo"
#define IDD_DATA_JATUHTEMPO_VERSION    "1"

class UpdateJatuhTempoRunnable : public Runnable
{
public:
    UpdateJatuhTempoRunnable(CoreEngine *_engine) : engine(_engine) {}

    void run() {
        DatabasePtr db = engine->database();
        db->exec(QString("SELECT `value` FROM `info` WHERE `key`='%1'").arg(IDD_DATA_JATUHTEMPO_KEY));

        QString version;
        if (db->next())
            version = db->value(0).toString();

        if (version >= IDD_DATA_JATUHTEMPO_VERSION)
            return;

        QString filename = engine->workingDirectory() + "/data/jatuhtempo.csv";
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QTextStream in(&file);
        QString header = in.readLine();
        QHash<QString, int> headerHash;
        {
            QStringList headerSplit = engine->common()->split(header, ';');
            for (int i=0; i < headerSplit.count(); i++)
                headerHash[headerSplit[i]] = i;
        }

        qint64 contentPos = in.pos();
        int totalLine = 0;
        while (!in.atEnd()) {
            totalLine++;
            in.readLine();
        }

        in.seek(contentPos);

        db->compatibility()->truncateTable("jatuhtempo");
        db->compatibility()->beginTransaction();

        while (!in.atEnd()) {
            QString dataLine = in.readLine().trimmed();
            if (dataLine.isEmpty())
                continue;

            int bulan(0), tahun(0);
            QDate potput, pph, ppn, pphop, pphbdn;

            QStringList dataList = engine->common()->split(dataLine, ';');
            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_BULAN)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_BULAN]];
                if (!value.isEmpty())
                    bulan = value.toInt();
            }

            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_TAHUN)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_TAHUN]];
                if (!value.isEmpty())
                    tahun = value.toInt();
            }

            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_POTPUT)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_POTPUT]];
                if (!value.isEmpty())
                    potput = QDate::fromString(value, "dd/MM/yyyy");
            }

            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_PPH)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_PPH]];
                if (!value.isEmpty())
                    pph = QDate::fromString(value, "dd/MM/yyyy");
            }

            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_PPN)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_PPN]];
                if (!value.isEmpty())
                    ppn = QDate::fromString(value, "dd/MM/yyyy");
            }

            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_PPH_OP)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_PPH_OP]];
                if (!value.isEmpty())
                    pphop = QDate::fromString(value, "dd/MM/yyyy");
            }

            if (headerHash.contains(IDD_JATUHTEMPO_HEADER_PPH_BADAN)) {
                QString value = dataList[headerHash[IDD_JATUHTEMPO_HEADER_PPH_BADAN]];
                if (!value.isEmpty())
                    pphbdn = QDate::fromString(value, "dd/MM/yyyy");
            }

            db->prepare("INSERT INTO `jatuhtempo` VALUES(:col0, :col1, :col2, :col3, :col4, :col5, :col6)");
            db->bindValue(":col0", bulan);
            db->bindValue(":col1", tahun);
            db->bindValue(":col2", potput);
            db->bindValue(":col3", pph);
            db->bindValue(":col4", ppn);
            db->bindValue(":col5", pphop);
            db->bindValue(":col6", pphbdn);
            db->exec();
        }

        db->compatibility()->endTransaction();
        file.close();

        db->exec(QString("INSERT INTO `info` VALUES('%1', '%2')").arg(IDD_DATA_JATUHTEMPO_KEY).arg(IDD_DATA_JATUHTEMPO_VERSION));
    }

    CoreEngine *engine;
};

class DatabaseUpdateJatuhTempo : public Process
{
public:
    DatabaseUpdateJatuhTempo(CoreEngine *engine) : mEngine(engine) {}

    void run() {
        mEngine->runSync(new UpdateJatuhTempoRunnable(mEngine));
    }

private:
    CoreEngine *mEngine;
};

#endif // DATABASEUPDATEJATUHTEMPO_H
