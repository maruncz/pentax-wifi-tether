#ifndef DATEQUEUE_H
#define DATEQUEUE_H

#include <QMutex>
#include <QQueue>
#include <QTimer>

class FileInfo;
class QObject;

class DateQueue : public QObject
{
    Q_OBJECT
public:
    explicit DateQueue(QObject *parent = nullptr);

    void enqueue(FileInfo *fileinfo);
    bool urlExists(FileInfo *info) const;

    void start();
    void stop();

signals:

    void ready(FileInfo *fileinfo);

public slots:

private slots:

    void fetch();

    void on_fetched(FileInfo *fileinfo);
    void on_fetch_error(FileInfo *fileinfo);

private:
    QQueue<FileInfo *> queue;
    mutable QMutex enqMutex;
    QTimer timer;
    QAtomicInt fetching{0};
};

#endif // DATEQUEUE_H
