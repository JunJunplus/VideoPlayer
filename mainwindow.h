#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "VideoDecoder.h"
#include "VideoShow.h"
#include <QTimer>
#include <QResizeEvent>

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
private slots:
    void onPlay();
private:
    QTimer* m_timer;
    VideoDecoder* m_video;
    VideoShow* m_videoShow;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
