// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "environment.h"

Environment::Environment(int interval, QObject *parent) : QObject(parent), m_interval(interval), m_open(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Environment::timeout);
    m_timer->setInterval(m_interval);

    m_temp = new QFile("/sys/class/hwmon/hwmon2/temp1_input");
    m_humidity = new QFile("/sys/class/hwmon/hwmon2/humidity1_input");
    if (m_temp->open(QFile::ReadOnly)) {
        m_tempStream = new QTextStream(m_temp);
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ":" << m_temp->errorString();
    }
    if (m_humidity->open(QFile::ReadOnly)) {
        m_humidityStream = new QTextStream(m_humidity);
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ":" << m_humidity->errorString();
    }
    m_open = m_temp->isOpen() && m_humidity->isOpen();
}

Environment::~Environment()
{
    delete m_timer;
}

void Environment::go()
{
    m_timer->start();
    timeout();
}

void Environment::timeout()
{
    int t;
    int h;

    if (m_open) {
        m_tempStream->seek(0);
        t = m_tempStream->readAll().toInt();
        m_humidityStream->seek(0);
        h = m_humidityStream->readAll().toInt();
        emit humidity(h / 1000);
        emit temperature((t / 1000) * 1.8 + 32);
    }
}
