#include "Slider.h"
#include <QDebug>

Slider::Slider(QWidget *parent)
    : QWidget{parent}
{
    m_slider = new QSlider(this);
    m_slider->setRange(0, 1000);
    m_slider->setValue(0);
    m_slider->setOrientation(Qt::Horizontal);

    m_slider->setGeometry(100, 60, 600, 30);
    this->setStyleSheet(R"(

)");
    m_slider->setStyleSheet(R"(
QSlider::groove:horizontal{
        height:12px;
        left:0px;
        right:0px;
        border:0px;
        border-radius:6px;
        background:rgba(0,0,0,50);
}
QSlider::handle:horizontal{
        background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 white, stop:1 rgba(80,166,234,1));
        width: 16px;
        height: 16px;
        border-radius:11px;
        border: 3px solid #ffffff;
        margin-top:-5px;
        margin-left:0px;
        margin-bottom:-5px;
        margin-right:0px;
}
QSlider::sub-page:horizontal{
    border-radius:6px;
    background:rgba(80,166,234,1);
}

)");
    m_slider->show();
    m_left = new QLabel(this);
    m_left->setText(GetDuration(m_current));
    m_left->show();
    m_right = new QLabel(this);
    m_right->setText(GetDuration(m_total - m_current));
    m_right->show();
    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(m_left);
    m_layout->addWidget(m_slider);
    m_layout->addWidget(m_right);
    m_layout->setEnabled(true);

    connect(m_slider, &QSlider::sliderReleased, this, &Slider::seek);
    connect(m_slider, &QSlider::sliderReleased, this, [this](){ m_drag = true; });
    connect(m_slider, &QSlider::sliderPressed, this, [this](){ m_drag = false; });
}

void Slider::SetTotal(qint64 timestamp)
{
    m_total = timestamp;
    m_right->setText(GetDuration(m_total - m_current));
}

void Slider::Setcurrent(qint64 timestamp)
{
    m_current = timestamp;
    if (m_drag) {
        m_left->setText(GetDuration(m_current));
        SetSliderValue((double)m_current / (double)m_total * 1000);
    }
}

void Slider::SetSliderValue(int value)
{
    m_slider->setValue(value);
}

void Slider::seek()
{
    int value = m_slider->value();
    qDebug() << "value: " << value;
    m_current = ((double)value / 1000.0) * m_total;
    m_left->setText(GetDuration(m_current));
    m_right->setText(GetDuration(m_total - m_current));
    emit onSeekStamp(m_current);
}

QString Slider::GetDuration(qint64 timestamp)
{
    int hours = timestamp / 3600;
    int minutes = (timestamp % 3600) / 60;
    int seconds = timestamp % 60;

    QString timeString = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));

    return timeString;
}
