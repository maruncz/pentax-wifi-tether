#include "fileinfo.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <utility>

FileInfo::FileInfo(QUrl url, QObject *parent)
    : QObject(parent), fileUrl(std::move(url))
{
    connect(&networkManager, &QNetworkAccessManager::finished, this,
            &FileInfo::on_networkManager_finished);
}

void FileInfo::on_networkManager_finished(QNetworkReply *reply)
{
    if (reply == infoReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QString datetime  = obj.value(QStringLiteral("datetime")).toString();
            if (!datetime.isEmpty())
            {
                QStringList l1 = datetime.split('T');
                filePath =
                    l1.front() + '/' + fileUrl.toString().split('/').back();
                emit readyForDownload(this);
            }
            else
            {
                qDebug() << reply->readAll();
            }
        }
    }
}

bool FileInfo::getDownloaded() const
{
    return downloaded;
}

void FileInfo::setDownloaded(bool value)
{
    downloaded = value;
}

QString FileInfo::getFilePath() const
{
    return filePath;
}

QString FileInfo::getFileDir() const
{
    return filePath.split('/', QString::SkipEmptyParts).front();
}

QString FileInfo::getFileName() const
{
    return filePath.split('/', QString::SkipEmptyParts).back();
}

void FileInfo::getDate()
{
    infoReply =
        networkManager.get(QNetworkRequest(QUrl(fileUrl.toString() + "/info")));
}

bool FileInfo::operator==(const FileInfo &rhs) const
{
    return (fileUrl == rhs.fileUrl);
}

bool FileInfo::operator<(const FileInfo &rhs) const
{
    return (fileUrl < rhs.fileUrl);
}

QUrl FileInfo::getFileUrl() const
{
    return fileUrl;
}

void FileInfo::setFileUrl(const QUrl &value)
{
    fileUrl = value;
}
