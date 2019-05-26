#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fileinfo.h"
#include <QFile>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
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
    ~MainWindow();

private slots:
    void on_buttonConnect_clicked();

    void on_networkManager_finished(QNetworkReply *reply);

    void on_buttonStart_clicked();

    void on_timer_timeout();

    void on_buttonStop_clicked();

    void on_files_changed();

    void on_pushButton_clicked();

    void on_download_ready_read();

signals:

    void filesChanged();

private:
    using downloadPath = QPair<QString, QString>;

    bool alreadyDownloaded(FileInfo *fileinfo);

    Ui::MainWindow *ui;

    QNetworkAccessManager networkManager{this};

    QNetworkReply *connectReply{nullptr};
    QNetworkReply *listReply{nullptr};
    QNetworkReply *fileReply{nullptr};

    QTimer timer{this};
    bool run{false};

    QList<FileInfo *> fileList;
    QList<FileInfo *> downloadedList;
    QString savePrefix;
    QFile *file;
};

#endif // MAINWINDOW_H
