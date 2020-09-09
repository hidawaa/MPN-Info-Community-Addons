#ifndef DATABASEUPDATEKLU_H
#define DATABASEUPDATEKLU_H

#include <QFile>

#include <coreengine.h>
#include <runnable.h>

#define IDD_KLU_HEADER_KODE                                     "KODE"
#define IDD_KLU_HEADER_NAMA                                     "NAMA"
#define IDD_KLU_HEADER_SEKTOR                                   "SEKTOR"

#define IDD_DATA_KLU_KEY        "data.klu"
#define IDD_DATA_KLU_VERSION    "1"

class UpdateKluRunnable : public Runnable
{
public:
    UpdateKluRunnable(CoreEngine *_engine) : engine(_engine) {}

    void run() {
        DatabasePtr db = engine->database();
        db->exec(QString("SELECT `value` FROM `info` WHERE `key`='%1'").arg(IDD_DATA_KLU_KEY));

        QString version;
        if (db->next())
            version = db->value(0).toString();

        if (version >= IDD_DATA_KLU_VERSION)
            return;

        QString filename = engine->workingDirectory() + "/data/klu.csv";
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

        db->compatibility()->truncateTable("klu");
        db->compatibility()->beginTransaction();

        while (!in.atEnd()) {
            QString dataLine = in.readLine().trimmed();
            if (dataLine.isEmpty())
                continue;

            QString kode, nama, sektor;

            QStringList dataList = engine->common()->split(dataLine, ';');
            if (headerHash.contains(IDD_KLU_HEADER_KODE)) {
                QString value = dataList[headerHash[IDD_KLU_HEADER_KODE]];
                if (!value.isEmpty())
                    kode = value;
            }

            if (headerHash.contains(IDD_KLU_HEADER_NAMA)) {
                QString value = dataList[headerHash[IDD_KLU_HEADER_NAMA]];
                if (!value.isEmpty())
                    nama = value;
            }

            if (headerHash.contains(IDD_KLU_HEADER_SEKTOR)) {
                QString value = dataList[headerHash[IDD_KLU_HEADER_SEKTOR]];
                if (!value.isEmpty())
                    sektor = value;
            }

            db->prepare("INSERT INTO `klu` VALUES(:col0, :col1, :col2)");
            db->bindValue(":col0", kode);
            db->bindValue(":col1", nama);
            db->bindValue(":col2", sektor);
            db->exec();
        }

        db->compatibility()->endTransaction();
        file.close();

        db->exec(QString("INSERT INTO `info` VALUES('%1', '%2')").arg(IDD_DATA_KLU_KEY).arg(IDD_DATA_KLU_VERSION));
    }

    CoreEngine *engine;
};

class DatabaseUpdateKlu : public Process
{
public:
    DatabaseUpdateKlu(CoreEngine *engine) : mEngine(engine) {}

    void run() {
        mEngine->runSync(new UpdateKluRunnable(mEngine));
    }

private:
    CoreEngine *mEngine;
};

#endif // DATABASEUPDATEKLU_H
