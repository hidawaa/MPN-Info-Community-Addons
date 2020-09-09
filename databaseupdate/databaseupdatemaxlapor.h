#ifndef DATABASEUPDATEMAXLAPOR_H
#define DATABASEUPDATEMAXLAPOR_H

#include <QFile>

#include <coreengine.h>
#include <runnable.h>

#define IDD_MAXLAPOR_HEADER_BULAN                               "BULAN"
#define IDD_MAXLAPOR_HEADER_TAHUN                               "TAHUN"
#define IDD_MAXLAPOR_HEADER_PPH                                 "PPH"
#define IDD_MAXLAPOR_HEADER_PPN                                 "PPN"
#define IDD_MAXLAPOR_HEADER_PPH_OP                              "PPHOP"
#define IDD_MAXLAPOR_HEADER_PPH_BADAN                           "PPHBDN"

#define IDD_DATA_MAXLAPOR_KEY        "data.maxlapor"
#define IDD_DATA_MAXLAPOR_VERSION    "1"

class UpdateMaxLaporRunnable : public Runnable
{
public:
    UpdateMaxLaporRunnable(CoreEngine *_engine) : engine(_engine) {}

    void run() {
        DatabasePtr db = engine->database();
        db->exec(QString("SELECT `value` FROM `info` WHERE `key`='%1'").arg(IDD_DATA_MAXLAPOR_KEY));

        QString version;
        if (db->next())
            version = db->value(0).toString();

        if (version >= IDD_DATA_MAXLAPOR_VERSION)
            return;

        QString filename = engine->workingDirectory() + "/data/maxlapor.csv";
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

        db->compatibility()->truncateTable("maxlapor");
        db->compatibility()->beginTransaction();

        while (!in.atEnd()) {
            QString dataLine = in.readLine().trimmed();
            if (dataLine.isEmpty())
                continue;

            int bulan(0), tahun(0);
            QDate pph, ppn, pphOp, pphBdn;

            QStringList dataList = engine->common()->split(dataLine, ';');
            if (headerHash.contains(IDD_MAXLAPOR_HEADER_BULAN)) {
                QString value = dataList[headerHash[IDD_MAXLAPOR_HEADER_BULAN]];
                if (!value.isEmpty())
                    bulan = value.toInt();
            }

            if (headerHash.contains(IDD_MAXLAPOR_HEADER_TAHUN)) {
                QString value = dataList[headerHash[IDD_MAXLAPOR_HEADER_TAHUN]];
                if (!value.isEmpty())
                    tahun = value.toInt();
            }

            if (headerHash.contains(IDD_MAXLAPOR_HEADER_PPH)) {
                QString value = dataList[headerHash[IDD_MAXLAPOR_HEADER_PPH]];
                if (!value.isEmpty())
                    pph = QDate::fromString(value, "dd/MM/yyyy");
            }

            if (headerHash.contains(IDD_MAXLAPOR_HEADER_PPN)) {
                QString value = dataList[headerHash[IDD_MAXLAPOR_HEADER_PPN]];
                if (!value.isEmpty())
                    ppn = QDate::fromString(value, "dd/MM/yyyy");
            }

            if (headerHash.contains(IDD_MAXLAPOR_HEADER_PPH_OP)) {
                QString value = dataList[headerHash[IDD_MAXLAPOR_HEADER_PPH_OP]];
                if (!value.isEmpty())
                    pphOp = QDate::fromString(value, "dd/MM/yyyy");
            }


            if (headerHash.contains(IDD_MAXLAPOR_HEADER_PPH_BADAN)) {
                QString value = dataList[headerHash[IDD_MAXLAPOR_HEADER_PPH_BADAN]];
                if (!value.isEmpty())
                    pphBdn = QDate::fromString(value, "dd/MM/yyyy");
            }

            db->prepare("INSERT INTO `maxlapor` VALUES(:col0, :col1, :col2, :col3, :col4, :col5)");
            db->bindValue(":col0", bulan);
            db->bindValue(":col1", tahun);
            db->bindValue(":col2", pph);
            db->bindValue(":col3", ppn);
            db->bindValue(":col4", pphOp);
            db->bindValue(":col5", pphBdn);
            db->exec();
        }

        db->compatibility()->endTransaction();
        file.close();

        db->exec(QString("INSERT INTO `info` VALUES('%1', '%2')").arg(IDD_DATA_MAXLAPOR_KEY).arg(IDD_DATA_MAXLAPOR_VERSION));
    }

    CoreEngine *engine;
};

class DatabaseUpdateMaxLapor : public Process
{
public:
    DatabaseUpdateMaxLapor(CoreEngine *engine) : mEngine(engine) {}

    void run() {
        mEngine->runSync(new UpdateMaxLaporRunnable(mEngine));
    }

private:
    CoreEngine *mEngine;
};

#endif // DATABASEUPDATEMAXLAPOR_H
