#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include "datequeue.h"
#include "downloadqueue.h"
#include "fileinfo.h"
#include <QAbstractTableModel>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

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

private:
    void on_networkManager_finished(QNetworkReply *reply);
    void on_timer_timeout();
    void update(FileInfo *fileinfo, const QVector<int> &roles = QVector<int>());
    void setDownloaded(FileInfo *info, bool value);

    QNetworkAccessManager networkManager{this};
    QNetworkReply *listReply{nullptr};

    QString savePrefix;

    DateQueue pendingList;
    DownloadQueue downloadList;
    QList<FileInfo *> fileList;
    QTimer timer{this};
};

#endif // FILELISTMODEL_H
