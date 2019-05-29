#ifndef DATEQUEUE_H
#define DATEQUEUE_H

#include "fileinfo.h"
#include <QAtomicInt>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QTimer>

class DateQueue : public QObject
{
    Q_OBJECT
public:
    explicit DateQueue(QObject *parent = nullptr);

    void enqueue(FileInfo *fileinfo);
    bool urlExists(FileInfo *info) const;

signals:

    void enqueued(FileInfo *fileinfo);
    void ready(FileInfo *fileinfo);

public slots:

private slots:

    void fetch();

    void on_fetched(FileInfo *fileinfo);
    void on_fetch_error(FileInfo *fileinfo);

private:
    QQueue<FileInfo *> queue;
    QMutex enqMutex;
    QMutex deqMutex;
    QTimer timer;
    QAtomicInt fetching{0};
};

#endif // DATEQUEUE_H
