#include "countdownwidget.h"

CountdownWidget::CountdownWidget(QWidget *parent) : QWidget(parent)
{
    m_remain = 60;
    m_layout = new QHBoxLayout();
    m_countdown = new QLabel();
    m_layout->addWidget(m_countdown);
    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &CountdownWidget::timeout);
    setLayout(m_layout);
}

CountdownWidget::~CountdownWidget()
{
}

void CountdownWidget::go()
{
    m_timer->setInterval(1000);
    m_countdown->setNum(60);
    m_timer->start();
}

void CountdownWidget::timeout()
{
    if (m_remain > 0)
        m_countdown->setNum(--m_remain);
    else {
        m_remain = 60;
        m_timer->stop();
        emit finished();
    }
}
