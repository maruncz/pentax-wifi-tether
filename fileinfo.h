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
    explicit FileInfo(const QUrl &url, QObject *parent = nullptr);

    QUrl getFileUrl() const;
    void setFileUrl(const QUrl &value);

    QString getFilePath() const;

    void getDate();

    bool operator==(const FileInfo &rhs);

signals:

public slots:

private slots:

    void on_networkManager_finished(QNetworkReply *reply);

private:
    QUrl fileUrl;
    QString filePath;
    QNetworkReply *infoReply;
    QNetworkAccessManager networkManager{this};
};

#endif // FILEINFO_H
