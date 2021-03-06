#include "mainwindow.h"
#include "filelistmodel.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&networkManager, &QNetworkAccessManager::finished, this,
            &MainWindow::onNetworkManagerFinished);
    listModel = new FileListModel(this);
    sortModel = new QSortFilterProxyModel(this);
    sortModel->setSourceModel(listModel);
    connect(listModel, &FileListModel::rowsInserted, sortModel,
            [this](const QModelIndex & /*parent*/, int /*first*/,
                   int /*last*/) { sortModel->sort(0); });
    ui->tableView->setModel(sortModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    connect(listModel, &FileListModel::downloadProgress, this,
            &MainWindow::onDownloadProgress);
    connect(listModel, &FileListModel::globalDownloadProgress, this,
            &MainWindow::onGlobalDownloadProgress);
    connect(listModel, &FileListModel::connectionLost, this,
            &MainWindow::onConnectionLost);

    timeout.setSingleShot(true);
    timeout.setInterval(1000);
    connect(&timeout, &QTimer::timeout, this, &MainWindow::onTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    ui->labelConnection->setText(QStringLiteral("Connecting..."));
    connectReply = networkManager.get(
        QNetworkRequest(QUrl(QStringLiteral("http://192.168.0.1/v1/props"))));
    timeout.start();
}

void MainWindow::onNetworkManagerFinished(QNetworkReply *reply)
{
    if (reply == connectReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QJsonValue err    = obj.value(QStringLiteral("errCode"));
            if (err.toInt() == 200)
            {
                ui->buttonStart->setEnabled(true);
            }
            else
            {
                qDebug() << reply->readAll();
            }
            QJsonValue model = obj.value(QStringLiteral("model"));
            ui->labelConnection->setText("Connected to: " + model.toString());
        }
        else
        {
            qDebug() << "Connection error: " << reply->errorString();
            ui->labelConnection->setText("Connection error: " +
                                         reply->errorString());
        }
        timeout.stop();
    }
}

void MainWindow::on_buttonStart_clicked()
{
    listModel->setRun(true);
    ui->buttonStop->setEnabled(true);
    ui->buttonStart->setEnabled(false);
    ui->buttonConnect->setEnabled(false);
}

void MainWindow::on_buttonStop_clicked()
{
    listModel->setRun(false);
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(true);
    ui->buttonConnect->setEnabled(true);
}

void MainWindow::onDownloadProgress(const QString &name, int percent,
                                    double rate)
{
    ui->progressDownload->setValue(percent);
    ui->progressDownload->setFormat(name + ": %p% (" + QString::number(rate) +
                                    "kB/s)");
}

void MainWindow::onGlobalDownloadProgress(int downloadedFiles, int totalFiles)
{
    ui->progressGlobal->setMaximum(totalFiles);
    ui->progressGlobal->setValue(downloadedFiles);
    ui->progressGlobal->setFormat("Files: " + QString::number(downloadedFiles) +
                                  '/' + QString::number(totalFiles) + " %p%");
}

void MainWindow::on_buttonDest_clicked()
{
    savePrefix = QFileDialog::getExistingDirectory(
        this, QStringLiteral("select download location"));
    ui->lineEdit->setText(savePrefix);
    listModel->setSavePrefix(savePrefix);
    QFileInfo dir(savePrefix);
    if (dir.isDir() && dir.isWritable())
    {
        ui->buttonConnect->setEnabled(true);
    }
    else
    {
        ui->buttonConnect->setEnabled(false);
    }
}

void MainWindow::onConnectionLost()
{
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(false);
    ui->buttonConnect->setEnabled(true);
    ui->labelConnection->setText(QStringLiteral("Connection lost"));
}

void MainWindow::onTimeout()
{
    connectReply->abort();
    ui->labelConnection->setText(QStringLiteral("Connection timed out"));
}
