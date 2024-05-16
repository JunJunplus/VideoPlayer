#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>

class Slider : public QWidget
{
    Q_OBJECT
public:
    explicit Slider(QWidget *parent = nullptr);
    void SetTotal(qint64 timestamp);
    void Setcurrent(qint64 timestamp);
    void SetSliderValue(int value);
signals:
    void onSeekStamp(qint64 stamp);
private slots:
    void seek();

private:
    QString GetDuration(qint64 timestamp);
private:
    QLabel* m_left;
    QSlider* m_slider;
    QLabel* m_right;
    QHBoxLayout* m_layout;
    int64_t m_total = 0;
    int64_t m_current = 0;
    bool m_drag = true;
};

#endif // SLIDER_H
