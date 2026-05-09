#ifndef ADDONS_H
#define ADDONS_H

#include <coreengine.h>
#include <interface.h>

#include "downloader.h"

class DownloaderObject : public Object
{
public:
    DownloaderObject() {
        mDownloader.reset(new Downloader);
    }

    QStringList keys() { return QStringList() << "setProxy" << "setHeader" << "responseHeader" << "get" << "post"; }
    QVariant exec(const QString &key, const QVariant &arg) {
        QVariantMap dataMap = arg.toMap();

        if (key == "setProxy") {
            QString hostname = dataMap["hostname"].toString();
            quint16 port = quint16(dataMap["port"].toUInt());
            QString username = dataMap["username"].toString();
            QString password = dataMap["password"].toString();

            QNetworkProxy proxy;
            proxy.setHostName(hostname);
            proxy.setPort(port);
            proxy.setUser(username);
            proxy.setPassword(password);

            mDownloader->setProxy(proxy);
        }
        else if (key == "setHeader") {
            QByteArray key = dataMap["key"].toByteArray();
            QByteArray data = dataMap["data"].toByteArray();

            mDownloader->setHeader(key, data);
        }
        else if (key == "responseHeader") {
            QUrl url = dataMap["url"].toUrl();

            return mDownloader->responseHeader();
        }
        else if (key == "get") {
            QUrl url = dataMap["url"].toUrl();

            return mDownloader->get(url);
        }
        else if (key == "post") {
            QUrl url = dataMap["url"].toUrl();
            QByteArray data = dataMap["data"].toByteArray();

            return mDownloader->post(url, data);
        }

        return QVariant();
    }

private:
    QScopedPointer<Downloader> mDownloader;
};

class DownloaderAddOn : public AddOn
{
public:
    QString name() { return "downloader"; }
    QString group() { return "Utility"; }
    QString title() { return "Network Downloader"; }
    AddOnTypes type() { return AddOnObject; }
    int loadFlags() { return 0; }
    int permission() { return 0; }
    ObjectPtr newObject() { return ObjectPtr(new DownloaderObject); }
};

#endif // ADDONS_H
