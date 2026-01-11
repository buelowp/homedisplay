#include "countdown.h"

CountdownWidget::CountdownWidget(QWidget *parent) : QWidget(parent)
{
    m_remain = 60;
    m_layout = new QHBoxLayout();
    m_countdown = new QLabel();
    m_layout->addWidget(m_countdown);
    setLayout(m_layout);
}

CountdownWidget::~CountdownWidget()
{
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
