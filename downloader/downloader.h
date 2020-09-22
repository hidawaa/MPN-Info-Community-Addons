#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QSslError>
#include <QNetworkProxy>

class QNetworkAccessManager;
class QNetworkReply;
class Downloader : public QObject
{
    Q_OBJECT
public:
    Downloader(QObject *parent = nullptr);

    void setProxy(const QNetworkProxy &proxy);

    void setHeader(const QByteArray &key, const QByteArray &value);
    const QVariantMap &responseHeader();

    QByteArray get(const QUrl &url, bool *ok = nullptr);
    QByteArray post(const QUrl &url, const QByteArray &data = QByteArray(), bool *ok = nullptr);
    void abort();

private:
    QByteArray download(const QString &method, const QUrl &url, const QByteArray &data = QByteArray(), bool *ok = nullptr);

private slots:
    void onFinished();

    void onSslErrors(const QList<QSslError> &errors);

private:
    QNetworkAccessManager *mNam;
    QNetworkReply *mReply;
    QVariantMap mRequestHeaderMap;
    QVariantMap mResponseHeaderMap;
    bool mRunning;
};

#endif // DOWNLOADER_H
