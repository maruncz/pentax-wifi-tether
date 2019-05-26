#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDebug>
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
    connect(&timer, &QTimer::timeout, this, &MainWindow::on_timer_timeout);
    timer.setSingleShot(true);
    connect(this, &MainWindow::filesChanged, this,
            &MainWindow::on_files_changed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    connectReply =
        networkManager.get(QNetworkRequest(QUrl("http://192.168.0.1/v1/ping")));
}

void MainWindow::on_networkManager_finished(QNetworkReply *reply)
{
    if (reply == connectReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QJsonValue err    = obj.value("errCode");
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
    else if (reply == listReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            bool changed      = false;
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QJsonArray dirs   = obj.value("dirs").toArray();
            for (const auto d : dirs)
            {
                QJsonArray files = d.toObject().value("files").toArray();
                for (const auto f : files)
                {
                    QUrl url = "http://192.168.0.1/v1/photo/" +
                               d.toObject().value("name").toString() + "/" +
                               f.toString();

                    FileInfo *file = new FileInfo(url, this);

                    auto it = std::find(fileList.begin(), fileList.end(), file);

                    if (it != fileList.end())
                    {
                        fileList.append(file);
                        changed = true;
                    }
                    else
                    {
                        delete file;
                    }
                }
            }
            if (changed)
            {
                emit filesChanged();
            }
        }
    }
}

void MainWindow::on_buttonStart_clicked()
{
    timer.start(1000);
    run = true;
}

void MainWindow::on_timer_timeout()
{
    listReply = networkManager.get(
        QNetworkRequest(QUrl("http://192.168.0.1/v1/photos")));
}

void MainWindow::on_buttonStop_clicked()
{
    run = false;
}

void MainWindow::on_files_changed()
{
    for (const auto f : fileList)
    {
        if (downloadedList.contains(f))
        {
            continue;
        }
        if (alreadyDownloaded(f))
        {
            downloadedList.append(f);
            continue;
        }
    }
}

bool MainWindow::alreadyDownloaded(FileInfo *fileinfo)
{
    QString filepath  = fileinfo->getFilePath();
    auto split        = filepath.split('/', QString::SkipEmptyParts);
    QString dir       = split.front();
    QString file      = split.back();
    split             = file.split('.');
    QString filename  = split.front();
    QString extension = split.back();
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            QString tmp = dir + '/' +
                          (j ? filename.toLower() : filename.toUpper()) + '.' +
                          (i ? extension.toLower() : extension.toUpper());
            if (QFile::exists(savePrefix + '/' + tmp))
            {
                return true;
            }
        }
    }
    return false;
}

void MainWindow::on_pushButton_clicked()
{
    savePrefix =
            QFileDialog::getExistingDirectory(this, "select download location");
}

void MainWindow::on_download_ready_read()
{
    file->write(fileReply->readAll());
}
