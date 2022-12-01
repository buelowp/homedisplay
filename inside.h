// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#pragma once

#include <QtCore/QtCore>
#include <sht31d.h>
extern "C" {
    #include <linux/i2c.h>
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
}

class Inside : public QObject
{
    Q_OBJECT

public:
    Inside(int frequency = 1000 * 60, QObject *parent = nullptr);
    ~Inside();

    void go() { m_timer->start(); timeout(); }
    bool isOpen() { return m_sht31d->isOpen(); }

public slots:
    void timeout();

signals:
    void temperature(float temp);
    void humidity(float humidity);

private:
    bool instantiateSHT();
    bool instantiateTCS();
    bool available(uint8_t device, uint8_t address);

    SHT31D *m_sht31d;
    QTimer *m_timer;
    float m_temperature;
    float m_humidity;
    bool m_isSHT;
    bool m_isTCS;
    bool m_available;
    int m_frequency;
};

