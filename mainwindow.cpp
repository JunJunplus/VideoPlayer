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
    m_videoShow = new VideoShow(this);
    //m_videoShow->setGeometry(0, 0, 800, 600);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onPlay);
    m_video->openVideo("D:\\testvideo\\4.mp4");
    m_timer->setInterval(33);
    m_timer->start();

    m_slider = new Slider(this);
    QSize size = this->size();
   // m_slider->setMinimumHeight(300);
    m_slider->setGeometry(0, size.height() - 200, size.width(), 200);
    m_slider->SetTotal(m_video->GetTotalTime());
    connect(m_slider, &Slider::onSeekStamp, m_video, &VideoDecoder::Seek);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    //m_video->SetSize(size.width(), size.height());
    m_videoShow->setGeometry(0, 0, size.width(), size.height());
    m_slider->setGeometry(0, size.height() - 200, size.width(), 200);
}

void MainWindow::onPlay()
{
    // QImage image = m_video->Decode();
    // if (!image.isNull()) {

    //     ui->label->setPixmap(QPixmap::fromImage(image));
    //     //m_timer->stop();
    //     qDebug() << "end";
    // }

    AVFrame* frame = m_video->PopFrame();

    if (frame) {
        qDebug() << "pop";
        m_slider->Setcurrent(m_video->GetCurStamp(frame));
        m_videoShow->UpdataTexture(frame);
        av_frame_unref(frame);
    }
}
