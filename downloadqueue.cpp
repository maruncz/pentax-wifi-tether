#include "downloadqueue.h"
#include "fileinfo.h"

DownloadQueue::DownloadQueue(QObject *parent) : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &DownloadQueue::fetch);
    timer.setSingleShot(false);
}

void DownloadQueue::enqueue(FileInfo *fileinfo)
{
    QMutexLocker locker(&enqMutex);
    qDebug() << "enqueue download: " << fileinfo->getFileUrl();
    queue.enqueue(fileinfo);
    connect(fileinfo, &FileInfo::fileDownloaded, this,
            &DownloadQueue::onDownloaded);
    connect(fileinfo, &FileInfo::downloadError, this,
            &DownloadQueue::onDownloadError);
}

void DownloadQueue::fetch()
{
    if (fetching)
    {
        return;
    }
    fetching = 1;
    if (queue.isEmpty())
    {
        fetching = 0;
        return;
    }
    if (queue.head()->isDownloaded())
    {
        onDownloaded(queue.head());
        return;
    }
    auto file = queue.head();
    connect(file, &FileInfo::downloadProgress, this,
            &DownloadQueue::onDownloadProgress);
    file->download(savePrefix);
}

void DownloadQueue::onDownloaded(FileInfo *fileinfo)
{
    QMutexLocker locker(&enqMutex);
    qDebug() << "downloaded: " << fileinfo->getFileUrl();
    auto idx  = queue.indexOf(fileinfo);
    auto file = queue.at(idx);
    file->disconnect(this);
    queue.removeAt(idx);
    emit downloaded(fileinfo);
    fetching = 0;
}

void DownloadQueue::onDownloadError(FileInfo *fileinfo)
{
    qDebug() << "download error: " << fileinfo->getFileUrl();
    fetching = 0;
}

void DownloadQueue::onDownloadProgress(const QString &name, int percent,
                                       double rate)
{
    emit downloadProgress(name, percent, rate);
}

void DownloadQueue::setSavePrefix(const QString &value)
{
    savePrefix = value;
}

void DownloadQueue::start()
{
    timer.start(100);
}

void DownloadQueue::stop()
{
    QMutexLocker locker(&enqMutex);
    timer.stop();
    if (!queue.isEmpty())
    {
        queue.head()->abort();
    }
}
