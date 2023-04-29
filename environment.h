// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QtCore/QtCore>

class Environment : public QObject
{
    Q_OBJECT

public:
    Environment(int interval = (1000 * 60), QObject *parent = nullptr);
    ~Environment();

public slots:
    void go();
    void timeout();

signals:
    void temperature(double temp);
    void humidity(double humidity);

private:
    QString m_hwmon;
    QFile *m_temp;
    QFile *m_humidity;
    QTextStream *m_tempStream;
    QTextStream *m_humidityStream;
    QTimer *m_timer;
    int m_interval;
    bool m_open;
};

#endif // ENVIRONMENT_H
