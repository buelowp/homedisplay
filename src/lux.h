// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef LUX_H
#define LUX_H

#include <QtCore/QtCore>
#include <tsl2561.h>

class Lux : public QObject
{
    Q_OBJECT

public:
    Lux(uint8_t device = 4, uint8_t address = 0x39, QObject *parent = nullptr);
    ~Lux();
    void setInterval(int interval = 1000) { m_interval = interval; }
    long max() { return m_max; }
    long min() { return m_min; }
    bool isOpen() { return (m_tsl != nullptr) && m_open; }

public slots:
    void go();
    void timeout();

signals:
    void lux(long l);

private:
    int map(int x, int in_min, int in_max, int out_min, int out_max);

    QTimer *m_timer;
    QString m_device;
    uint8_t m_address;
    tsl2561_t *m_tsl;
    int m_interval;
    long m_max;
    long m_min;
    long m_open;
    QFile m_backlight;
    int m_maxBrightness;
    int m_lastVal;
};

#endif // LUX_H
