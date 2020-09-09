#ifndef DATABASEUPDATEKANTOR_H
#define DATABASEUPDATEKANTOR_H

#include <QFile>

#include <coreengine.h>
#include <runnable.h>

#define IDD_KANTOR_HEADER_KANWIL                                "KANWIL"
#define IDD_KANTOR_HEADER_KPP                                   "KPP"
#define IDD_KANTOR_HEADER_NAMA                                  "NAMA"

#define IDD_DATA_KANTOR_KEY        "data.kantor"
#define IDD_DATA_KANTOR_VERSION    "1"

class UpdateKantorRunnable : public Runnable
{
public:
    UpdateKantorRunnable(CoreEngine *_engine) : engine(_engine) {}

    void run() {
        DatabasePtr db = engine->database();
        db->exec(QString("SELECT `value` FROM `info` WHERE `key`='%1'").arg(IDD_DATA_KANTOR_KEY));

        QString version;
        if (db->next())
            version = db->value(0).toString();

        if (version >= IDD_DATA_KANTOR_VERSION)
            return;

        QString filename = engine->workingDirectory() + "/data/kantor.csv";
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

        db->compatibility()->truncateTable("kantor");
        db->compatibility()->beginTransaction();

        while (!in.atEnd()) {
            QString dataLine = in.readLine().trimmed();
            if (dataLine.isEmpty())
                continue;

            QString kanwil, kpp, nama;

            QStringList dataList = engine->common()->split(dataLine, ';');
            if (headerHash.contains(IDD_KANTOR_HEADER_KANWIL)) {
                QString value = dataList[headerHash[IDD_KANTOR_HEADER_KANWIL]];
                if (!value.isEmpty())
                    kanwil = value;
            }

            if (headerHash.contains(IDD_KANTOR_HEADER_KPP)) {
                QString value = dataList[headerHash[IDD_KANTOR_HEADER_KPP]];
                if (!value.isEmpty())
                    kpp = value;
            }

            if (headerHash.contains(IDD_KANTOR_HEADER_NAMA)) {
                QString value = dataList[headerHash[IDD_KANTOR_HEADER_NAMA]];
                if (!value.isEmpty())
                    nama = value;
            }

            db->prepare("INSERT INTO `kantor` VALUES(:col0, :col1, :col2)");
            db->bindValue(":col0", kanwil);
            db->bindValue(":col1", kpp);
            db->bindValue(":col2", nama);
            db->exec();
        }

        db->compatibility()->endTransaction();
        file.close();

        db->exec(QString("INSERT INTO `info` VALUES('%1', '%2')").arg(IDD_DATA_KANTOR_KEY).arg(IDD_DATA_KANTOR_VERSION));
    }

    CoreEngine *engine;
};

class DatabaseUpdateKantor : public Process
{
public:
    DatabaseUpdateKantor(CoreEngine *engine) : mEngine(engine) {}

    void run() {
        mEngine->runSync(new UpdateKantorRunnable(mEngine));
    }

private:
    CoreEngine *mEngine;
};

#endif // DATABASEUPDATEKANTOR_H
