// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef SHT31D_H
#define SHT31D_H

#include <QtCore/QtCore>
#include <sht31d.h>

class Inside : public QObject
{
    Q_OBJECT

public:
    Inside(QObject *parent = nullptr);
    ~Inside();

    void go() { m_timer->start(); timeout(); }
    bool isOpen() { return m_sht31d->isOpen(); }

public slots:
    void timeout();

signals:
    void temperature(float temp);
    void humidity(float humidity);

private:
    SHT31D *m_sht31d;
    QTimer *m_timer;
    float m_temperature;
    float m_humidity;
};

#endif // SHT31D_H
