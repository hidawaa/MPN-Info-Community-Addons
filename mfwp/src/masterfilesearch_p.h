#ifndef MASTERFILESEARCH_P_H
#define MASTERFILESEARCH_P_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include <QVariant>

#include <coreengine.h>
#include <runnable.h>

class MasterfileSearchFunction : public QObject
{
    Q_OBJECT

public:
    MasterfileSearchFunction(CoreEngine *engine, QObject *parent = nullptr);

    void search(const QVariantMap &args);

signals:
    void rowsReady(const QList<QVariantList> &rows);
    void finished(int count);

public slots:
    void onRowsReady(const QList<QVariantList> &rows) {
        emit rowsReady(rows);
    }
    void onFinished(int count) {
        emit finished(count);
    }

private:
    CoreEngine *mEngine;
};

class MasterfileSearchRunnable : public Runnable
{
public:
    MasterfileSearchRunnable(CoreEngine *engine, MasterfileSearchFunction *fn, const QVariantMap &args);

    void run() override;

private:
    CoreEngine *mEngine;
    MasterfileSearchFunction *mFunction;
    QVariantMap mArgs;
};

#endif // MASTERFILESEARCH_P_H
