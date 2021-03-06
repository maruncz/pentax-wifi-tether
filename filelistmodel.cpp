#include "filelistmodel.h"
#include "fileinfo.h"
#include <QBrush>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

FileListModel::FileListModel(QObject *parent) : QAbstractTableModel(parent)
{
    connect(&networkManager, &QNetworkAccessManager::finished, this,
            &FileListModel::onNetworkManagerFinished);
    connect(&timer, &QTimer::timeout, this, &FileListModel::onTimerTimeout);
    timer.setSingleShot(false);
    connect(&pendingList, &DateQueue::ready, &downloadList,
            &DownloadQueue::enqueue);
    connect(&pendingList, &DateQueue::ready, this, &FileListModel::append);
    connect(&downloadList, &DownloadQueue::downloaded, this,
            &FileListModel::setDownloaded);
    connect(&downloadList, &DownloadQueue::downloadProgress, this,
            &FileListModel::onDownloadProgress);
    connect(&downloadList, &DownloadQueue::downloaded, this,
            &FileListModel::onFileDownloaded);
    timeout.setSingleShot(true);
    timeout.setInterval(15000);
    connect(&timeout, &QTimer::timeout, this, &FileListModel::onTimeout);
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    if (index.column() != 0)
    {
        return QVariant();
    }
    switch (role)
    {
        case Qt::DisplayRole: return fileList.at(index.row())->getFileName();

        case Qt::BackgroundRole:
            if (fileList.at(index.row())->isDownloaded())
            {
                return QBrush(Qt::green);
            }
            return QVariant();
    }
    return QVariant();
}

QVariant FileListModel::headerData(int /*section*/, Qt::Orientation orientation,
                                   int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            if (orientation == Qt::Horizontal)
            {
                return "Files";
            }
    }
    return QVariant();
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return fileList.size();
}

int FileListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return 1;
}

void FileListModel::append(FileInfo *value)
{
    beginInsertRows(QModelIndex(), fileList.size(), fileList.size());
    fileList.append(value);
    endInsertRows();
    onFileDownloaded(nullptr);
}

bool FileListModel::urlExists(FileInfo *const info) const
{
    auto it = std::find_if(fileList.begin(), fileList.end(),
                           [&info](const FileInfo *p) { return *p == *info; });
    if (it != fileList.end())
    {
        return true;
    }
    return pendingList.urlExists(info);
}

void FileListModel::setDownloaded(FileInfo *info)
{
    auto idx  = fileList.indexOf(info);
    auto midx = index(idx, 0);
    emit dataChanged(midx, midx, QVector<int>(Qt::DisplayRole));
}

void FileListModel::onNetworkManagerFinished(QNetworkReply *reply)
{
    if (reply == listReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QJsonArray dirs   = obj.value(QStringLiteral("dirs")).toArray();
            for (const auto d : dirs)
            {
                QJsonArray files =
                    d.toObject().value(QStringLiteral("files")).toArray();
                for (const auto f : files)
                {
                    auto ext = f.toString().split('.').back();
                    // skip video files
                    if (ext == QLatin1String("MOV"))
                    {
                        continue;
                    }
                    QUrl url =
                        "http://192.168.0.1/v1/photos/" +
                        d.toObject().value(QStringLiteral("name")).toString() +
                        "/" + f.toString();
                    auto *file = new FileInfo(url, this);

                    if (!urlExists(file))
                    {
                        qDebug() << "new: " << url;
                        pendingList.enqueue(file);
                    }
                    else
                    {
                        delete file;
                    }
                }
            }
            timeout.start();
        }
    }
}

void FileListModel::onTimerTimeout()
{
    listReply = networkManager.get(
        QNetworkRequest(QUrl(QStringLiteral("http://192.168.0.1/v1/photos"))));
}

void FileListModel::onTimeout()
{
    setRun(false);
    emit connectionLost();
}

void FileListModel::update(FileInfo *const fileinfo, const QVector<int> &roles)
{
    auto idx  = fileList.indexOf(fileinfo);
    auto midx = index(idx, 0);
    emit dataChanged(midx, midx, roles);
}

bool FileListModel::getRun() const
{
    return timer.isActive();
}

void FileListModel::setRun(bool value)
{
    if (value)
    {
        pendingList.start();
        downloadList.start();
        timer.start(1000);
    }
    else
    {
        pendingList.stop();
        downloadList.stop();
        timer.stop();
    }
}

void FileListModel::onDownloadProgress(const QString &name, int percent,
                                       double rate)
{
    emit downloadProgress(name, percent, rate);
}

void FileListModel::onFileDownloaded(FileInfo * /*fileinfo*/)
{
    int numDownloaded = 0;
    Q_FOREACH (auto file, fileList)
    {
        if (file->isDownloaded())
        {
            ++numDownloaded;
        }
    }
    emit globalDownloadProgress(numDownloaded, fileList.size());
}

QString FileListModel::getSavePrefix() const
{
    return savePrefix;
}

void FileListModel::setSavePrefix(const QString &value)
{
    savePrefix = value;
    downloadList.setSavePrefix(value);
}
