// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "clockdisplay.h"

ClockDisplay::ClockDisplay(QFrame *parent) : QFrame(parent)
{
    m_layout = new QGridLayout();
    m_primaryClock = new QLabel();
    m_primaryClock->setScaledContents(true);
    m_primaryClock->setAlignment(Qt::AlignCenter);
    m_primaryDate = new QLabel();
    m_primaryDate->setAlignment(Qt::AlignCenter);
    m_primaryDate->setScaledContents(true);
    m_temperature = new QLabel();
    m_temperature->setAlignment(Qt::AlignCenter);
    m_temperature->setScaledContents(true);
    m_humidity = new QLabel();
    m_humidity->setScaledContents(true);
    m_humidity->setAlignment(Qt::AlignCenter);
    m_rain = new QLabel();
    m_rain->setAlignment(Qt::AlignCenter);
    m_rain->setScaledContents(true);
    m_uvIndex = new QLabel();
    m_uvIndex->setAlignment(Qt::AlignCenter);
    m_uvIndex->setScaledContents(true);

    QFont c("Roboto-Regular", 36);
    QFont l("Roboto-Regular", 28);
    QFont p("Roboto-Regular", 100);
    QFont d("Roboto-Regular", 32);

    m_primaryClock->setFont(p);
    m_primaryDate->setFont(d);
    m_temperature->setFont(c);
    m_humidity->setFont(c);
    m_rain->setFont(l);
    m_uvIndex->setFont(l);

    m_layout->addWidget(m_primaryClock, 0, 0, 1, 4);
    m_layout->addWidget(m_temperature, 2, 0, 1, 2);
    m_layout->addWidget(m_humidity, 2, 2, 1, 2);
    m_layout->addWidget(m_rain, 3, 0, 1, 2);
    m_layout->addWidget(m_uvIndex, 3, 2, 1, 2);
    m_layout->addWidget(m_primaryDate, 4, 0, 1, 4);
    setLayout(m_layout);

    m_clockTimer = new QTimer();
    connect(m_clockTimer, &QTimer::timeout, this, &ClockDisplay::clockTimeout);
    m_clockTimer->setInterval(500);
    m_clockTimer->start();

    m_time = QDateTime::currentDateTime();
    m_primaryClock->setText(m_time.time().toString("h:mm A"));
    m_primaryDate->setText(m_time.date().toString("dddd MMMM d, yyyy"));
}

ClockDisplay::~ClockDisplay()
{
}

void ClockDisplay::showEvent(QShowEvent* e)
{
}


void ClockDisplay::clockTimeout()
{
    QDateTime now = QDateTime::currentDateTime();

    if (now.date() != m_time.date()) {
        m_primaryDate->setText(now.date().toString("dddd MMMM d, yyyy"));
    }
    if (now.time() != m_time.time()) {
        m_primaryClock->setText(now.time().toString("h:mm A"));
    }
    m_time = now;
}

void ClockDisplay::updateDisplay(QString &topic, QJsonObject &object)
{
    if (topic == "weather/rainfall") {
        if (object.contains("today")) {
            m_rain->setText(QString("%1 in").arg(object["today"].toDouble(), 0, 'f', 1));
        }
    }
    else if (topic == "weather/conditions") {
        if (object.contains("environment")) {
            QJsonObject env = object["environment"].toObject();
            m_temperature->setText(QString("%1%2").arg(env["farenheit"].toDouble(), 0, 'f', 1).arg(QChar(176)));
            m_humidity->setText(QString("%1%").arg(env["humidity"].toDouble(), 0, 'f', 1));
        }
    }
    else if (topic == "weather/light") {
        if (object.contains("uv")) {
            int uv = object["uv"].toInt();
            switch (uv) {
                case 0:
                case 1:
                case 2:
                case 3:
                    m_uvIndex->setText(QString("UV Index: <span style=\"color:green;\">%1</span>").arg(uv));
                    break;
                case 4:
                case 5:
                case 6:
                    m_uvIndex->setText(QString("UV Index: <span style=\"color:yellow;\">%1</span>").arg(uv));
                    break;
                default:
                    m_uvIndex->setText(QString("UV Index: <span style=\"color:red;\">%1</span>").arg(uv));
                    break;
            }
        }
    }
}
