#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

class FileListModel;
class QNetworkReply;
class QSortFilterProxyModel;

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
    void on_download_progress(const QString &name, int percent, double rate);
    void on_global_download_progress(int downloadedFiles, int totalFiles);
    void on_buttonDest_clicked();

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager networkManager{this};
    QNetworkReply *connectReply{nullptr};
    FileListModel *listModel{nullptr};
    QSortFilterProxyModel *sortModel{nullptr};
    QString savePrefix;
};

#endif // MAINWINDOW_H
