// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "bigclock.h"

BigClock::BigClock(QFrame *parent) : QFrame(parent)
{
    m_layout = new QGridLayout();
    m_primaryClock = new QLabel();
    m_primaryClock->setScaledContents(true);
    m_primaryClock->setAlignment(Qt::AlignCenter);
    m_primaryDate = new QLabel();
    m_primaryDate->setAlignment(Qt::AlignCenter);
    m_primaryDate->setScaledContents(true);

    QFont c("Roboto-Regular", 130);
    c.setBold(true);
    QFont d("Roboto-Regular", 34);

    m_primaryClock->setFont(c);
    m_primaryDate->setFont(d);

    m_layout->addWidget(m_primaryClock, 0, 0, 3, 1);
    m_layout->addWidget(m_primaryDate, 3, 0, 1, 1);

    setLayout(m_layout);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &BigClock::updateClock);
    m_updateTimer->setInterval(1000);
}

BigClock::~BigClock()
{
}

void BigClock::showEvent(QShowEvent* e)
{
    qDebug() << __PRETTY_FUNCTION__;
    updateClock();
}

void BigClock::begin()
{
    m_updateTimer->start();
}

void BigClock::end()
{
    m_updateTimer->stop();
}

void BigClock::updateClock()
{
    QDateTime now = QDateTime::currentDateTime();

    if (now.date() != m_time.date()) {
        m_primaryDate->setText(now.date().toString("dddd MMMM d, yyyy"));
    }
    if (now.time() != m_time.time()) {
        m_primaryClock->setText(now.time().toString("h:mm"));
    }
    m_time = now;
}
