#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "Slider.h"
#include "VideoDecoder.h"
#include "VideoShow.h"
#include <QDragEnterEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private slots:
    void onPlay();
private:
    QTimer* m_timer;
    VideoDecoder* m_video;
    VideoShow* m_videoShow;
    Slider* m_slider = nullptr;
    QVBoxLayout* m_vLayout;
    QHBoxLayout* m_hLayoyt;
    QString m_filePath;
    QSize m_size;
    Ui::MainWindow *ui;    
};
#endif // MAINWINDOW_H
