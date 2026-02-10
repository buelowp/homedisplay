// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "lux.h"

Lux::Lux(uint8_t device, uint8_t address, QObject *parent) : QObject(parent), m_address(address), m_max(0), m_interval(250)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");

    m_open = false;
    m_maxBrightness = 255;
    m_min = 1;
    m_max = 255;
    m_lastVal = -1;

    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &Lux::timeout);
    m_timer->setInterval(m_interval);

    m_device = QString("/dev/i2c-%1").arg(device);
    if ((m_tsl = tsl2561_init(address, m_device.toLocal8Bit().data())) != NULL) {
        tsl2561_enable_autogain(m_tsl);
        tsl2561_set_integration_time(m_tsl, TSL2561_INTEGRATION_TIME_13MS);
        m_open = true;
    }

    if (settings.contains("backlight")) {
        QFileInfo fi(settings.value("backlight").toString());
        if (fi.isDir() || fi.exists()) {
            QString brightness = fi.absoluteFilePath() + "/brightness";
            QString max = fi.absoluteFilePath() + "/max_brightness";
            QFile mb(max);
            if (mb.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray line = mb.readAll();
                m_maxBrightness = line.toInt();
                qDebug() << __PRETTY_FUNCTION__ << ": m_maxBrightness:" << m_maxBrightness;
            }

            m_backlight.setFileName(brightness);
            if (m_backlight.open(QIODevice::WriteOnly | QIODevice::Text)) {
                m_open = m_open & true;
                QTextStream ts(&m_backlight);
                ts << m_maxBrightness;
            }
        }
        else {
            qWarning() << __PRETTY_FUNCTION__ << ": baclight entry must be parent dir for the backlight (" << fi.absoluteFilePath();
        }
    }
    else {
        qWarning() << __PRETTY_FUNCTION__ << ": Unable to find a backlight entry in the config file";
        m_open = false;
    }
}

Lux::~Lux()
{
    m_timer->stop();
}

int Lux::map(int x, int in_min, int in_max, int out_min, int out_max)
{
    int out = out_max - out_min;
    int in = (in_max - in_min) + out_min;
    if (in > 0) {
        return (x - in_min) * out / in;
    }
    return m_maxBrightness;
}

void Lux::go()
{
    timeout();

    if (m_open)
        m_timer->start();
}

void Lux::timeout()
{
    QDateTime now = QDateTime::currentDateTime();
    long l = tsl2561_lux(m_tsl);

    if (now.time().hour() >= 7 && now.time().hour() <= 20) {
        return;
    }

    int b = map(static_cast<int>(l), m_min, m_max, 1, m_maxBrightness);
    if (b == 0)
        b = 1;

    if (b != m_lastVal) {
        QTextStream ts(&m_backlight);
        ts << b;
    }
}
