// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "inside.h"

Inside::Inside(QObject *parent) : QObject(parent)
{
    m_sht31d = new SHT31D(4);
    m_timer = new QTimer(this);
    m_timer->setInterval(1000 * 60);
    connect(m_timer, &QTimer::timeout, this, &Inside::timeout);
}

Inside::~Inside()
{
    m_timer->stop();
}

void Inside::timeout()
{
    if (m_sht31d->isOpen()) {
        if (m_sht31d->values(m_temperature, m_humidity) == SHT31D::sht31dreturn::SHT31_OK) {
            emit (temperature(m_temperature));
            emit (humidity(m_humidity));
        }
    }
}
