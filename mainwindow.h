#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "VideoDecoder.h"
#include <QTimer>
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
private slots:
    void onPlay();
private:
    QTimer* m_timer;
    VideoDecoder *m_video;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
