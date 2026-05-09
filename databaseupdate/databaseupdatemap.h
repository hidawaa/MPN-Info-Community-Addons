#ifndef DATABASEUPDATEMAP_H
#define DATABASEUPDATEMAP_H

#include <QFile>

#include <coreengine.h>
#include <runnable.h>

#define IDD_MAP_HEADER_KDMAP                                    "KDMAP"
#define IDD_MAP_HEADER_KDBAYAR                                  "KDBAYAR"
#define IDD_MAP_HEADER_SEKTOR                                   "SEKTOR"
#define IDD_MAP_HEADER_URAIAN                                   "URAIAN"

#define IDD_DATA_MAP_KEY        "data.map"
#define IDD_DATA_MAP_VERSION    "1"

class UpdateMapRunnable : public Runnable
{
public:
    UpdateMapRunnable(CoreEngine *_engine) : engine(_engine) {}

    void run() {
        DatabasePtr db = engine->database();
        db->exec(QString("SELECT `value` FROM `info` WHERE `key`='%1'").arg(IDD_DATA_MAP_KEY));

        QString version;
        if (db->next())
            version = db->value(0).toString();

        if (version >= IDD_DATA_MAP_VERSION)
            return;

        QString filename = engine->workingDirectory() + "/data/map.csv";
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

        db->compatibility()->truncateTable("map");
        db->compatibility()->beginTransaction();

        while (!in.atEnd()) {
            QString dataLine = in.readLine().trimmed();
            if (dataLine.isEmpty())
                continue;

            QString kdmap, kdbayar, uraian;
            int sektor(-1);

            QStringList dataList = engine->common()->split(dataLine, ';');
            if (headerHash.contains(IDD_MAP_HEADER_KDMAP)) {
                QString value = dataList[headerHash[IDD_MAP_HEADER_KDMAP]];
                if (!value.isEmpty())
                    kdmap = value;
            }

            if (headerHash.contains(IDD_MAP_HEADER_KDBAYAR)) {
                QString value = dataList[headerHash[IDD_MAP_HEADER_KDBAYAR]];
                if (!value.isEmpty())
                    kdbayar = value;
            }

            if (headerHash.contains(IDD_MAP_HEADER_SEKTOR)) {
                QString value = dataList[headerHash[IDD_MAP_HEADER_SEKTOR]];
                if (!value.isEmpty())
                    sektor = value.toInt();
            }

            if (headerHash.contains(IDD_MAP_HEADER_URAIAN)) {
                QString value = dataList[headerHash[IDD_MAP_HEADER_URAIAN]];
                if (!value.isEmpty())
                    uraian = value;
            }

            db->prepare("INSERT INTO `map` VALUES(:col0, :col1, :col2, :col3)");
            db->bindValue(":col0", kdmap);
            db->bindValue(":col1", kdbayar);
            db->bindValue(":col2", sektor);
            db->bindValue(":col3", uraian);
            db->exec();
        }

        db->compatibility()->endTransaction();
        file.close();

        db->exec(QString("INSERT INTO `info` VALUES('%1', '%2')").arg(IDD_DATA_MAP_KEY).arg(IDD_DATA_MAP_VERSION));
    }

    CoreEngine *engine;
};

class DatabaseUpdateMap : public Process
{
public:
    DatabaseUpdateMap(CoreEngine *engine) : mEngine(engine) {}

    void run() {
        mEngine->runSync(new UpdateMapRunnable(mEngine));
    }

private:
    CoreEngine *mEngine;
};

#endif // DATABASEUPDATEMAP_H
