#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_video = new VideoDecoder();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onPlay);
    m_video->openVideo("D:\\4k.mp4");
    m_timer->setInterval(40);
    m_timer->start();
    ui->label->setFixedSize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPlay()
{
    QImage image = m_video->Decode();
    if (!image.isNull()) {

        ui->label->setPixmap(QPixmap::fromImage(image));
        //m_timer->stop();
        qDebug() << "end";
    }
}
