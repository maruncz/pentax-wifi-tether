#include "filelistmodel.h"
#include <QBrush>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <algorithm>

FileListModel::FileListModel(QObject *parent) : QAbstractTableModel(parent)
{
    connect(&networkManager, &QNetworkAccessManager::finished, this,
            &FileListModel::on_networkManager_finished);
    connect(this, &FileListModel::filesChanged, this,
            &FileListModel::on_files_changed);
    connect(&timer, &QTimer::timeout, this, &FileListModel::on_timer_timeout);
    timer.setSingleShot(true);
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    if (index.column() != 0)
    {
        return QVariant();
    }
    switch (role)
    {
        case Qt::DisplayRole: return fileList.at(index.row())->getFilePath();

        case Qt::BackgroundRole:
            if (fileList.at(index.row())->downloaded)
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
}

bool FileListModel::urlExists(FileInfo *const info) const
{
    auto it = std::find_if(fileList.begin(), fileList.end(),
                           [&info](const FileInfo *p) { return *p == *info; });
    return it != fileList.end();
}

void FileListModel::setDownloaded(FileInfo *info, bool value)
{
    info->downloaded = value;
    auto idx         = fileList.indexOf(info);
    auto midx        = index(idx, 0);
    emit dataChanged(midx, midx, QVector<int>(Qt::DisplayRole));
}

void FileListModel::on_networkManager_finished(QNetworkReply *reply)
{
    if (reply == listReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            bool changed      = false;
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QJsonArray dirs   = obj.value(QStringLiteral("dirs")).toArray();
            for (const auto d : dirs)
            {
                QJsonArray files =
                    d.toObject().value(QStringLiteral("files")).toArray();
                for (const auto f : files)
                {
                    QUrl url =
                        "http://192.168.0.1/v1/photos/" +
                        d.toObject().value(QStringLiteral("name")).toString() +
                        "/" + f.toString();
                    auto *file = new FileInfo(url, this);

                    if (!urlExists(file))
                    {
                        qDebug() << url;
                        connect(file, &FileInfo::readyForDownload, this,
                                &FileListModel::on_readyForDownload);
                        file->getDate();
                        changed = true;
                    }
                    else
                    {
                        delete file;
                    }
                }
            }
            if (!changed)
            {
                start();
            }
        }
        else
        {
            start();
        }
    }
}

void FileListModel::on_files_changed()
{
    Q_FOREACH (const auto f, fileList)
    {
        if (f->alreadyDownloaded(savePrefix))
        {
            continue;
        }
        if (!fileReply)
        {
            QDir dir(savePrefix + '/' + f->getFileDir());
            if (!dir.exists())
            {
                if (!dir.mkpath(dir.path()))
                {
                    qDebug() << "cannot create directory: " << dir.path();
                }
            }

            file = new QFile(savePrefix + '/' + f->getFilePath());
            if (!file->open(QIODevice::WriteOnly))
            {
                qDebug() << "cannot open file " << file->fileName() << ": "
                         << file->errorString();
                break;
            }
            fileReply = networkManager.get(QNetworkRequest(f->getFileUrl()));
            //            connect(fileReply, &QNetworkReply::finished, this,
            //                    &MainWindow::on_download_finished);
            connect(fileReply, &QNetworkReply::finished, this,
                    [f, this]() { this->on_download_finished(f); });
            connect(fileReply, &QNetworkReply::readyRead, this,
                    &FileListModel::on_download_ready_read);
            break;
        }
        break;
    }
    start();
}

void FileListModel::on_readyForDownload(FileInfo *fileinfo)
{
    append(fileinfo);
    update(fileinfo, QVector<int>(Qt::DisplayRole));
    emit filesChanged();
}

void FileListModel::on_download_finished(FileInfo *fileinfo)
{
    qDebug() << fileinfo->getFilePath() << " downloaded";
    fileinfo->downloaded = true;
    update(fileinfo, QVector<int>(Qt::BackgroundRole));
    file->close();
    delete file;
    file      = nullptr;
    fileReply = nullptr;
    emit filesChanged();
}

void FileListModel::on_download_ready_read()
{
    file->write(fileReply->readAll());
}

void FileListModel::on_timer_timeout()
{
    listReply = networkManager.get(
        QNetworkRequest(QUrl(QStringLiteral("http://192.168.0.1/v1/photos"))));
}

void FileListModel::start(int msec)
{
    if (run && (!fileReply) && (!timer.isActive()))
    {
        timer.start(msec);
    }
}

void FileListModel::update(FileInfo *const fileinfo, const QVector<int> &roles)
{
    auto idx  = fileList.indexOf(fileinfo);
    auto midx = index(idx, 0);
    emit dataChanged(midx, midx, roles);
}

bool FileListModel::getRun() const
{
    return run;
}

void FileListModel::setRun(bool value)
{
    run = value;
    start(1);
}

QString FileListModel::getSavePrefix() const
{
    return savePrefix;
}

void FileListModel::setSavePrefix(const QString &value)
{
    savePrefix = value;
}
