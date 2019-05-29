#include "fileinfo.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <cmath>
#include <utility>

FileInfo::FileInfo(QUrl url, QObject *parent)
    : QObject(parent), fileUrl(std::move(url))
{
    connect(&networkManager, &QNetworkAccessManager::finished, this,
            &FileInfo::on_networkManager_finished);
    timeout.setSingleShot(true);
    connect(&timeout, &QTimer::timeout, this, &FileInfo::on_timeout);
    rateTimer.setSingleShot(false);
    rateTimer.setInterval(1000);
    connect(&rateTimer, &QTimer::timeout, this,
            &FileInfo::on_rateTimer_timeout);
}

void FileInfo::on_networkManager_finished(QNetworkReply *reply)
{
    if (reply == infoReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QString datetime = obj.value(QStringLiteral("datetime")).toString();
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
            infoReply->deleteLater();
            infoReply = nullptr;
        }
        else
        {
            qDebug() << "date reply error: " << reply->error();
            emit dateFetchError(this);
        }
        timeout.stop();
    }
    else if (reply == downloadReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            file->close();
            downloaded = true;
            downloadReply->deleteLater();
            downloadReply = nullptr;
            emit fileDownloaded(this);
        }
        else
        {
            file->remove();
            downloadReply->deleteLater();
            downloadReply = nullptr;
            qDebug() << "download error: " << reply->errorString();
            emit downloadError(this);
        }
        file->deleteLater();
        file = nullptr;
        timeout.stop();
        rateTimer.stop();
    }
}

void FileInfo::on_download_ready_read()
{
    file->write(downloadReply->readAll());
}

void FileInfo::on_timeout()
{
    if (infoReply)
    {
        if (infoReply->isRunning())
        {
            infoReply->abort();
        }
    }
    else if (downloadReply)
    {
        if (downloadReply->isRunning())
        {
            downloadReply->abort();
        }
    }
}

void FileInfo::on_download_progress(qint64 bytesReceived, qint64 bytesTotal)
{
    timeout.start(15000);
    double pecrd =
        static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal);
    int percent  = static_cast<int>(std::round(100 * pecrd));
    bytesWritten = bytesReceived;
    emit downloadProgress(getFileName(), percent, downloadRate);
}

void FileInfo::on_rateTimer_timeout()
{
    downloadRate =
        static_cast<double>(bytesWritten - bytesWrittenPrevious) / 1000.0;
    bytesWrittenPrevious = bytesWritten;
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
    timeout.start(15000);
}

void FileInfo::download(const QString &savePrefix)
{
    if (alreadyDownloaded(savePrefix))
    {
        emit fileDownloaded(this);
        return;
    }
    QDir dir(savePrefix + '/' + getFileDir());
    if (!dir.exists())
    {
        if (!dir.mkpath(dir.path()))
        {
            qDebug() << "cannot create directory: " << dir.path();
        }
    }

    file = new QFile(savePrefix + '/' + getFilePath(), this);
    if (!file->open(QIODevice::WriteOnly))
    {
        qDebug() << "cannot open file " << file->fileName() << ": "
                 << file->errorString();
        return;
    }
    downloadReply = networkManager.get(QNetworkRequest(getFileUrl()));

    connect(downloadReply, &QNetworkReply::readyRead, this,
            &FileInfo::on_download_ready_read);
    connect(downloadReply, &QNetworkReply::downloadProgress, this,
            &FileInfo::on_download_progress);

    timeout.start(15000);
    rateTimer.start();
}

bool FileInfo::operator==(const FileInfo &rhs) const
{
    return (fileUrl == rhs.fileUrl);
}

bool FileInfo::operator<(const FileInfo &rhs) const
{
    return (fileUrl < rhs.fileUrl);
}

bool FileInfo::alreadyDownloaded(const QString &savePrefix)
{
    if (downloaded)
    {
        return true;
    }
    QString dir       = getFileDir();
    QString file      = getFileName();
    auto split        = file.split('.');
    QString filename  = split.front();
    QString extension = split.back();
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            QString tmp = dir + '/' +
                          (j ? filename.toLower() : filename.toUpper()) + '.' +
                          (i ? extension.toLower() : extension.toUpper());
            if (QFile::exists(savePrefix + '/' + tmp))
            {
                downloaded = true;
                return true;
            }
        }
    }
    return false;
}

QUrl FileInfo::getFileUrl() const
{
    return fileUrl;
}

void FileInfo::setFileUrl(const QUrl &value)
{
    fileUrl = value;
}
