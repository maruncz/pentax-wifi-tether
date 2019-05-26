#include "fileinfo.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

FileInfo::FileInfo(const QUrl &url, QObject *parent)
    : QObject(parent), fileUrl(url)
{
    getDate();
}

void FileInfo::on_networkManager_finished(QNetworkReply *reply)
{
    if (reply == infoReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QString datetime  = obj.value("datetime").toString();
            if (!datetime.isEmpty())
            {
                QStringList l1 = datetime.split('T');
                filePath =
                    l1.front() + '/' + fileUrl.toString().split('/').back();
            }
            else
            {
                qDebug() << reply->readAll();
            }
        }
    }
}

QString FileInfo::getFilePath() const
{
    return filePath;
}

void FileInfo::getDate()
{
    infoReply =
        networkManager.get(QNetworkRequest(QUrl(fileUrl.toString() + "/info")));
}

bool FileInfo::operator==(const FileInfo &rhs)
{
    return (fileUrl == rhs.fileUrl);
}

QUrl FileInfo::getFileUrl() const
{
    return fileUrl;
}

void FileInfo::setFileUrl(const QUrl &value)
{
    fileUrl = value;
}
