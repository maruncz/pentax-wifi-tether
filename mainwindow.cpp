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
    else if (reply == listReply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            bool changed      = false;
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj   = doc.object();
            QJsonArray dirs   = obj.value(QStringLiteral("dirs")).toArray();
            for (const auto d : dirs)
            {
                QJsonArray files =
                    d.toObject().value(QStringLiteral("files")).toArray();
                for (const auto f : files)
                {
                    QUrl url =
                        "http://192.168.0.1/v1/photos/" +
                        d.toObject().value(QStringLiteral("name")).toString() +
                        "/" + f.toString();
                    auto *file = new FileInfo(url, this);

                    auto it = std::find_if(
                        fileList.begin(), fileList.end(),
                        [&file](const FileInfo *p) { return *p == *file; });
                    if (it == fileList.end())
                    {
                        qDebug() << url;
                        file->getDate();
                        connect(file, &FileInfo::readyForDownload, this,
                                &MainWindow::on_readyForDownload);
                        changed = true;
                    }
                    else
                    {
                        delete file;
                    }
                }
            }
            if (run && (!changed))
            {
                timer.start(1000);
            }
        }
    }
    else if (reply == fileReply)
    {
    }
}

void MainWindow::on_buttonStart_clicked()
{
    timer.start(1000);
    run = true;
    ui->buttonStop->setEnabled(true);
    ui->buttonStart->setEnabled(false);
}

void MainWindow::on_timer_timeout()
{
    listReply = networkManager.get(
        QNetworkRequest(QUrl(QStringLiteral("http://192.168.0.1/v1/photos"))));
}

void MainWindow::on_buttonStop_clicked()
{
    run = false;
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(true);
}

void MainWindow::on_files_changed()
{
    Q_FOREACH (const auto f, fileList)
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
        if (!fileReply)
        {
            QDir dir(savePrefix + '/' + f->getFileDir());
            if (!dir.exists())
            {
                if (!dir.mkpath(dir.path()))
                {
                    qDebug() << "cannot create directory: " << dir.path();
                }
            }

            file = new QFile(savePrefix + '/' + f->getFilePath());
            if (!file->open(QIODevice::WriteOnly))
            {
                qDebug() << "cannot open file " << file->fileName() << ": "
                         << file->errorString();
                break;
            }
            fileReply = networkManager.get(QNetworkRequest(f->getFileUrl()));
            connect(fileReply, &QNetworkReply::finished, this,
                    &MainWindow::on_download_finished);
            connect(fileReply, &QNetworkReply::readyRead, this,
                    &MainWindow::on_download_ready_read);
            break;
        }
    }
    if (run && (!fileReply))
    {
        timer.start(1000);
    }
}

bool MainWindow::alreadyDownloaded(FileInfo *fileinfo)
{
    QString dir       = fileinfo->getFileDir();
    QString file      = fileinfo->getFileName();
    auto split        = file.split('.');
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
    savePrefix = QFileDialog::getExistingDirectory(
        this, QStringLiteral("select download location"));
    ui->lineEdit->setText(savePrefix);
}

void MainWindow::on_download_ready_read()
{
    file->write(fileReply->readAll());
}

void MainWindow::on_download_finished()
{
    qDebug() << file->fileName() << " downloaded";
    file->close();
    delete file;
    file      = nullptr;
    fileReply = nullptr;
    if (run)
    {
        timer.start(1000);
    }
}

void MainWindow::on_readyForDownload(FileInfo *fileinfo)
{
    fileList.append(fileinfo);
    emit filesChanged();
}
