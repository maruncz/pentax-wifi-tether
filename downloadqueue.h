#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include "fileinfo.h"
#include <QAtomicInt>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTimer>

class DownloadQueue : public QObject
{
    Q_OBJECT
public:
    explicit DownloadQueue(QObject *parent = nullptr);

    void enqueue(FileInfo *fileinfo);

    void setSavePrefix(const QString &value);

signals:

    void downloaded(FileInfo *fileinfo);

public slots:

    void fetch();

    void on_downloaded(FileInfo *fileinfo);
    void on_download_error(FileInfo *fileinfo);

private:
    QQueue<FileInfo *> queue;
    QMutex enqMutex;
    QMutex deqMutex;
    QTimer timer;
    QString savePrefix;
    QAtomicInt fetching{0};
};

#endif // DOWNLOADQUEUE_H
