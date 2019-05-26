#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include "fileinfo.h"
#include <QAbstractListModel>
#include <QObject>

class FileListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FileListModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent) const override;

    void setFileList(QList<FileInfo *> *value);

    void update(FileInfo *const info);

private:
    QList<FileInfo *> *fileList{nullptr};
};

#endif // FILELISTMODEL_H
