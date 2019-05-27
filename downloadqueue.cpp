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
}

void DownloadQueue::fetch()
{
    if (fetching || queue.isEmpty())
    {
        return;
    }
    fetching = 1;
    if(queue.head()->alreadyDownloaded(savePrefix))
    {
        on_downloaded(queue.head());
        return;
    }
    queue.head()->download(savePrefix);
}

void DownloadQueue::on_downloaded(FileInfo *fileinfo)
{
    qDebug() << "downloaded: " << fileinfo->getFileUrl();
    auto idx = queue.indexOf(fileinfo);
    queue.removeAt(idx);
    emit downloaded(fileinfo);
    fetching = 0;
}

void DownloadQueue::setSavePrefix(const QString &value)
{
    savePrefix = value;
}
