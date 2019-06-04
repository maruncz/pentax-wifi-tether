#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include "datequeue.h"
#include "downloadqueue.h"
#include <QAbstractTableModel>
#include <QNetworkAccessManager>

class FileInfo;
class QNetworkReply;
class QObject;

class FileListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit FileListModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int /*section*/, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    void append(FileInfo *value);

    bool urlExists(FileInfo *info) const;

    QString getSavePrefix() const;
    void setSavePrefix(const QString &value);

    bool getRun() const;
    void setRun(bool value);

signals:

    void downloadProgress(const QString &name, int percent, double rate);
    void globalDownloadProgress(int downloadedFiles, int totalFiles);
    void connectionLost();

private slots:

    void onDownloadProgress(const QString &name, int percent, double rate);
    void onFileDownloaded(FileInfo *fileinfo);
    void onNetworkManagerFinished(QNetworkReply *reply);
    void onTimerTimeout();
    void onTimeout();

private:
    void update(FileInfo *fileinfo, const QVector<int> &roles = QVector<int>());
    void setDownloaded(FileInfo *info);

    QNetworkAccessManager networkManager{this};
    QNetworkReply *listReply{nullptr};

    QString savePrefix;

    DateQueue pendingList;
    DownloadQueue downloadList;
    QList<FileInfo *> fileList;
    QTimer timer{this};
    QTimer timeout{this};
};

#endif // FILELISTMODEL_H
