#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QLabel>
#include <QDebug>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setAcceptDrops(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    m_size = event->size();
    //m_video->SetSize(size.width(), size.height());
    if (m_slider) {
        m_videoShow->setGeometry(0, 0, m_size.width(), m_size.height());
        m_slider->setGeometry(0, m_size.height() - 200, m_size.width(), 200);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->urls()[0].fileName().right(3).compare("mp4")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *qm = event->mimeData();
    m_filePath = qm->urls()[0].toLocalFile();

    qDebug() << "文件路径: " << m_filePath;
    m_video = new VideoDecoder();
    m_videoShow = new VideoShow(this);
    m_videoShow->show();
    //m_videoShow->setGeometry(0, 0, 800, 600);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onPlay);
    m_timer->stop();
    m_video->openVideo(m_filePath);
    m_timer->setInterval(15);
    m_timer->start();

    m_slider = new Slider(this);
    QSize size = this->size();
    // m_slider->setMinimumHeight(300);
    m_slider->setGeometry(0, size.height() - 200, size.width(), 200);
    m_slider->SetTotal(m_video->GetTotalTime());
    connect(m_slider, &Slider::onSeekStamp, m_video, &VideoDecoder::Seek);
    m_slider->show();
    m_videoShow->setGeometry(0, 0, size.width(), size.height());
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
