#ifndef FILEINFO_H
#define FILEINFO_H

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

class FileInfo : public QObject
{
    Q_OBJECT

    friend class FileListModel;

public:
    explicit FileInfo(QUrl url, QObject *parent = nullptr);

    QUrl getFileUrl() const;
    void setFileUrl(const QUrl &value);

    QString getFilePath() const;
    QString getFileDir() const;
    QString getFileName() const;

    void getDate();
    void download(const QString &savePrefix);
    bool alreadyDownloaded(const QString &savePrefix);

    bool operator==(const FileInfo &rhs) const;
    bool operator<(const FileInfo &rhs) const;

signals:

    void readyForDownload(FileInfo *fileinfo);
    void fileDownloaded(FileInfo *fileinfo);
    void dateFetchError(FileInfo *fileinfo);
    void downloadError(FileInfo *fileinfo);

public slots:

private slots:

    void on_networkManager_finished(QNetworkReply *reply);
    void on_download_ready_read();
    void on_download_finished();

private:
    QUrl fileUrl;
    QString filePath;
    QNetworkReply *infoReply{nullptr};
    QNetworkReply *downloadReply{nullptr};
    QNetworkAccessManager networkManager{this};
    QFile *file{nullptr};
    bool downloaded{false};
};

#endif // FILEINFO_H
