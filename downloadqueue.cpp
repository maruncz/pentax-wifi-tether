#include "downloadqueue.h"

DownloadQueue::DownloadQueue(QObject *parent) : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &DownloadQueue::fetch);
    timer.setSingleShot(false);
    timer.start(100);
}

void DownloadQueue::enqueue(FileInfo *fileinfo)
{
    QMutexLocker locker(&enqMutex);
    qDebug() << "enqueue download: " << fileinfo->getFileUrl();
    queue.enqueue(fileinfo);
    connect(fileinfo, &FileInfo::fileDownloaded, this,
            &DownloadQueue::on_downloaded);
    connect(fileinfo, &FileInfo::downloadError, this,
            &DownloadQueue::on_download_error);
}

void DownloadQueue::fetch()
{
    if (fetching || queue.isEmpty())
    {
        return;
    }
    fetching = 1;
    if (queue.head()->isDownloaded())
    {
        on_downloaded(queue.head());
        return;
    }
    auto file = queue.head();
    connect(file, &FileInfo::downloadProgress, this,
            &DownloadQueue::on_download_progress);
    file->download(savePrefix);
}

void DownloadQueue::on_downloaded(FileInfo *fileinfo)
{
    qDebug() << "downloaded: " << fileinfo->getFileUrl();
    auto idx  = queue.indexOf(fileinfo);
    auto file = queue.at(idx);
    file->disconnect(this);
    queue.removeAt(idx);
    emit downloaded(fileinfo);
    fetching = 0;
}

void DownloadQueue::on_download_error(FileInfo *fileinfo)
{
    qDebug() << "download error: " << fileinfo->getFileUrl();
    fetching = 0;
}

void DownloadQueue::on_download_progress(const QString &name, int percent,
                                         double rate)
{
    emit downloadProgress(name, percent, rate);
}

void DownloadQueue::setSavePrefix(const QString &value)
{
    savePrefix = value;
}
