#ifndef FILEINFO_H
#define FILEINFO_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

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

    bool operator==(const FileInfo &rhs) const;
    bool operator<(const FileInfo &rhs) const;

    bool getDownloaded() const;
    void setDownloaded(bool value);

signals:

    void readyForDownload(FileInfo *fileinfo);

public slots:

private slots:

    void on_networkManager_finished(QNetworkReply *reply);

private:
    QUrl fileUrl;
    QString filePath;
    QNetworkReply *infoReply{nullptr};
    QNetworkAccessManager networkManager{this};
    bool downloaded{false};
};

#endif // FILEINFO_H
