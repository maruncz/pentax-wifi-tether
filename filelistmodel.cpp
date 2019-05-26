#include "filelistmodel.h"
#include <QBrush>
#include <algorithm>

FileListModel::FileListModel(QObject *parent) : QAbstractListModel(parent) {}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole: return fileList->at(index.row())->getFilePath();
        case Qt::BackgroundRole:
            if (fileList->at(index.row())->getDownloaded())
            {
                return QBrush(Qt::green);
            }
    }
    return QVariant();
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    if (fileList)
    {
        return fileList->size();
    }
    return 0;
}

void FileListModel::setFileList(QList<FileInfo *> *value)
{
    fileList = value;
}

void FileListModel::update(FileInfo *const info)
{
    auto idx      = fileList->indexOf(info);
    auto modelidx = index(idx);
    emit dataChanged(modelidx, modelidx);
}
