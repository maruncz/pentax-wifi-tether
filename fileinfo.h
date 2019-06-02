#ifndef FILEINFO_H
#define FILEINFO_H

#include <QNetworkAccessManager>
#include <QTimer>

class QNetworkReply;
class QObject;

class FileInfo : public QObject
{
    Q_OBJECT
public:
    explicit FileInfo(QUrl url, QObject *parent = nullptr);

    QUrl getFileUrl() const;
    void setFileUrl(const QUrl &value);

    QString getFilePath() const;
    QString getFileDir() const;
    QString getFileName() const;

    void getDate();
    void download(const QString &savePrefix);

    bool operator==(const FileInfo &rhs) const;
    bool operator<(const FileInfo &rhs) const;

    bool isDownloaded() const;

    void abort();

signals:

    void readyForDownload(FileInfo *fileinfo);
    void fileDownloaded(FileInfo *fileinfo);
    void dateFetchError(FileInfo *fileinfo);
    void downloadError(FileInfo *fileinfo);
    void downloadProgress(const QString &name, int percent, double rate);

public slots:

private slots:

    void onNetworkManagerFinished(QNetworkReply *reply);

    void onTimeout();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void onRateTimerTimeout();

private:
    bool alreadyDownloaded(const QString &savePrefix);

    QUrl fileUrl;
    QString filePath;
    QNetworkReply *infoReply{nullptr};
    QNetworkReply *downloadReply{nullptr};
    QNetworkAccessManager networkManager{this};
    QTimer timeout;
    QTimer rateTimer;
    qint64 bytesWrittenPrevious{0};
    qint64 bytesWritten{0};
    double downloadRate{0.0};
    QString savePrefix;

    bool downloaded{false};
};

#endif // FILEINFO_H
