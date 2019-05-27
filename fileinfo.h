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

    bool operator==(const FileInfo &rhs) const;
    bool operator<(const FileInfo &rhs) const;

signals:

    void readyForDownload(FileInfo *fileinfo);

public slots:

private slots:

    void on_networkManager_finished(QNetworkReply *reply);

private:
    bool alreadyDownloaded(const QString &savePrefix);
    QUrl fileUrl;
    QString filePath;
    QNetworkReply *infoReply{nullptr};
    QNetworkAccessManager networkManager{this};
    bool downloaded{false};
};

#endif // FILEINFO_H
