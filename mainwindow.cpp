#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMediaPlayer>
#include <QNetworkRequest>
#include <QUrl>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&networkManager, &QNetworkAccessManager::finished, this,
            &MainWindow::on_networkManager_finished);
    listModel = new FileListModel(this);
    sortModel = new QSortFilterProxyModel(this);
    sortModel->setSourceModel(listModel);
    connect(listModel, &FileListModel::rowsInserted, sortModel,
            [this](const QModelIndex & /*parent*/, int /*first*/,
                   int /*last*/) { sortModel->sort(0); });
    ui->tableView->setModel(sortModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    connectReply = networkManager.get(
        QNetworkRequest(QUrl(QStringLiteral("http://192.168.0.1/v1/ping"))));
}

void MainWindow::on_networkManager_finished(QNetworkReply *reply)
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
        }
    }
}

void MainWindow::on_buttonStart_clicked()
{
    listModel->setRun(true);
    ui->buttonStop->setEnabled(true);
    ui->buttonStart->setEnabled(false);
}

void MainWindow::on_buttonStop_clicked()
{
    listModel->setRun(false);
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(true);
}

void MainWindow::on_pushButton_clicked()
{
    savePrefix = QFileDialog::getExistingDirectory(
        this, QStringLiteral("select download location"));
    ui->lineEdit->setText(savePrefix);
    listModel->setSavePrefix(savePrefix);
}

void MainWindow::on_lineEdit_editingFinished()
{
    savePrefix = ui->lineEdit->text();
    listModel->setSavePrefix(savePrefix);
}
