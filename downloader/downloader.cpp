#include "downloader.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

Downloader::Downloader(QObject *parent) :
    QObject(parent),
    mReply(nullptr),
    mRunning(false)
{
    mNam = new QNetworkAccessManager(this);
}

void Downloader::setProxy(const QNetworkProxy &proxy)
{
    mNam->setProxy(proxy);
}

void Downloader::setHeader(const QByteArray &key, const QByteArray &value)
{
    mRequestHeaderMap[key] = value;
}

const QVariantMap &Downloader::responseHeader()
{
    return mResponseHeaderMap;
}

QByteArray Downloader::get(const QUrl &url, bool *ok)
{
    return download("GET", url, QByteArray(), ok);
}

QByteArray Downloader::post(const QUrl &url, const QByteArray &data, bool *ok)
{
    return download("POST", url, data, ok);
}

QByteArray Downloader::download(const QString &method, const QUrl &url, const QByteArray &data, bool *ok)
{
    QNetworkRequest req(url);

    QMapIterator<QString, QVariant> headerIterator(mRequestHeaderMap);
    while (headerIterator.hasNext()) {
        headerIterator.next();
        req.setRawHeader(headerIterator.key().toUtf8(), headerIterator.value().toByteArray());
    }

    mResponseHeaderMap.clear();
    mRunning = true;
    bool _ok(false);
    QByteArray buffer;

    if (method.toLower() == "post") {
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        mReply = mNam->post(req, data);
    }
    else if (method.toLower() == "get")
        mReply = mNam->get(req);

    if (mReply) {
        connect(mReply, &QNetworkReply::finished, this, &Downloader::onFinished);
        connect(mReply, &QNetworkReply::sslErrors, this, &Downloader::onSslErrors);

        while (mRunning) {
            qApp->processEvents();
            QThread::msleep(10);
        }

        foreach (const QByteArray &header, mReply->rawHeaderList())
            mResponseHeaderMap[header] = mReply->rawHeader(header);

        _ok = (mReply->error() == QNetworkReply::NoError);
        if (_ok)
            buffer = mReply->readAll();
    }

    if (ok)
        *ok = _ok;

    mReply = nullptr;
    return buffer;
}

void Downloader::abort()
{
    if (!mReply)
        return;

    mReply->abort();
}

void Downloader::onFinished()
{
    mRunning = false;
}

void Downloader::onSslErrors(const QList<QSslError> &errors)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(QObject::sender());
    reply->ignoreSslErrors(errors);
}
