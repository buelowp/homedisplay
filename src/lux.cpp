// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "lux.h"

Lux::Lux(uint8_t device, uint8_t address, QObject *parent) : QObject(parent), m_address(address), m_max(0)
{
    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &Lux::timeout);
    m_timer->setInterval(1000);

    m_device = QString("/dev/i2c-%1").arg(device);
    if ((m_tsl = tsl2561_init(address, m_device.toLocal8Bit().data())) != NULL) {
        tsl2561_enable_autogain(m_tsl);
        tsl2561_set_integration_time(m_tsl, TSL2561_INTEGRATION_TIME_13MS);
    }
}

Lux::~Lux()
{
    m_timer->stop();
}

void Lux::go()
{
    m_timer->start();
}

void Lux::timeout()
{
    long l = 0;
    l = tsl2561_lux(m_tsl);
    if (l > m_max)
        m_max = l;

    if (l < m_min)
        m_min = l;

    emit lux(l);
}
