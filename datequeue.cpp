#include "datequeue.h"
#include <QMutexLocker>

DateQueue::DateQueue(QObject *parent) : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &DateQueue::fetch);
    timer.setSingleShot(false);
    timer.start(100);
}

void DateQueue::enqueue(FileInfo *fileinfo)
{
    QMutexLocker locker(&enqMutex);
    qDebug() << "enqueue date: " << fileinfo->getFileUrl();
    queue.enqueue(fileinfo);
    connect(fileinfo, &FileInfo::readyForDownload, this,
            &DateQueue::on_fetched);
    connect(fileinfo, &FileInfo::dateFetchError, this,
            &DateQueue::on_fetch_error);
    emit enqueued(fileinfo);
}

bool DateQueue::urlExists(FileInfo *info) const
{
    auto it = std::find_if(queue.begin(), queue.end(),
                           [&info](const FileInfo *p) { return *p == *info; });
    return it != queue.end();
}

void DateQueue::fetch()
{
    if (fetching || queue.isEmpty())
    {
        return;
    }
    fetching = 1;
    queue.head()->getDate();
}

void DateQueue::on_fetched(FileInfo *fileinfo)
{
    qDebug() << "fetched date: " << fileinfo->getFileUrl();
    auto idx = queue.indexOf(fileinfo);
    queue.removeAt(idx);
    emit ready(fileinfo);
    fetching = 0;
}

void DateQueue::on_fetch_error(FileInfo *fileinfo)
{
    qDebug() << "error fetching date: " << fileinfo->getFileUrl();
    fetching = 0;
}
