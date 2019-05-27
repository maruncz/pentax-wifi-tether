#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fileinfo.h"
#include "filelistmodel.h"
#include <QFile>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QTimer>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_buttonConnect_clicked();

    void on_networkManager_finished(QNetworkReply *reply);

    void on_buttonStart_clicked();

    void on_buttonStop_clicked();

    void on_pushButton_clicked();

    void on_lineEdit_editingFinished();

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager networkManager{this};
    QNetworkReply *connectReply{nullptr};
    FileListModel *listModel{nullptr};
    QSortFilterProxyModel *sortModel{nullptr};
    QString savePrefix;
};

#endif // MAINWINDOW_H
