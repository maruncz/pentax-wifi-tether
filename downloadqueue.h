#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <QMutex>
#include <QQueue>
#include <QTimer>

class FileInfo;
class QObject;

class DownloadQueue : public QObject
{
    Q_OBJECT
public:
    explicit DownloadQueue(QObject *parent = nullptr);

    void enqueue(FileInfo *fileinfo);

    void setSavePrefix(const QString &value);

    void start();
    void stop();

signals:

    void downloaded(FileInfo *fileinfo);
    void downloadProgress(const QString &name, int percent, double rate);

public slots:

    void fetch();

    void on_downloaded(FileInfo *fileinfo);
    void on_download_error(FileInfo *fileinfo);
    void on_download_progress(const QString &name, int percent, double rate);

private:
    QQueue<FileInfo *> queue;
    QMutex enqMutex;
    QTimer timer;
    QString savePrefix;
    QAtomicInt fetching{0};
};

#endif // DOWNLOADQUEUE_H
