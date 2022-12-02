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
    m_insideTemp = new QLabel();
    m_insideTemp->setAlignment(Qt::AlignCenter);
    m_insideTemp->setScaledContents(true);
    m_insideHumidity = new QLabel();
    m_insideHumidity->setAlignment(Qt::AlignCenter);
    m_insideHumidity->setScaledContents(true);
    m_inside = new QLabel("Inside");
    m_inside->setScaledContents(true);
    m_inside->setAlignment(Qt::AlignCenter);
    m_outside = new QLabel("Outside");
    m_outside->setScaledContents(true);
    m_outside->setAlignment(Qt::AlignCenter);

    QLabel *temp = new QLabel("Temperature");
    temp->setScaledContents(true);
    temp->setAlignment(Qt::AlignCenter);
    QLabel *hum = new QLabel("Humidity");
    hum->setScaledContents(true);
    hum->setAlignment(Qt::AlignCenter);

    QFont c("Roboto-Regular", 36);
    QFont l("Roboto-Regular", 22);
    QFont p("Roboto-Regular", 100);
    QFont d("Roboto-Regular", 32);
    QFont a("Roboto-Regular", 18);

    m_primaryClock->setFont(p);
    m_primaryDate->setFont(d);
    m_temperature->setFont(l);
    m_humidity->setFont(l);
    m_insideTemp->setFont(l);
    m_insideHumidity->setFont(l);
    m_inside->setFont(l);
    m_outside->setFont(l);
    temp->setFont(a);
    hum->setFont(a);
    m_primaryClock->setStyleSheet("QLabel { color : khaki; }");
    m_primaryDate->setStyleSheet("QLabel { color : khaki; }");

    m_layout->addWidget(m_primaryClock, 0, 0, 1, 6);
    m_layout->addWidget(temp, 2, 2, 1, 2);
    m_layout->addWidget(hum, 2, 4, 1, 2);
    m_layout->addWidget(m_outside, 3, 0, 1, 2);
    m_layout->addWidget(m_temperature, 3, 2, 1, 2);
    m_layout->addWidget(m_humidity, 3, 4, 1, 2);
    m_layout->addWidget(m_inside, 4, 0, 1, 2);
    m_layout->addWidget(m_insideTemp, 4, 2, 1, 2);
    m_layout->addWidget(m_insideHumidity, 4, 4, 1, 2);
    m_layout->addWidget(m_primaryDate, 5, 0, 1, 6);
    setLayout(m_layout);

    m_clockTimer = new QTimer();
    connect(m_clockTimer, &QTimer::timeout, this, &ClockDisplay::clockTimeout);
    m_clockTimer->setInterval(500);
    m_clockTimer->start();

    m_time = QDateTime::currentDateTime();
    m_primaryClock->setText(m_time.time().toString("h:mm A"));
    m_primaryDate->setText(m_time.date().toString("dddd MMMM d, yyyy"));

    m_env = new Environment();
    connect(m_env, &Environment::temperature , this, &ClockDisplay::temperature);
    connect(m_env, &Environment::humidity , this, &ClockDisplay::humidity);
    m_env->go();
}

ClockDisplay::~ClockDisplay()
{
}

void ClockDisplay::showEvent(QShowEvent* e)
{
}

void ClockDisplay::humidity(double humidity)
{
    m_insideHumidity->setText(QString("%1%").arg(humidity, 0, 'f', 1));
}

void ClockDisplay::temperature(double temp)
{
    m_insideTemp->setText(QString("%1%2").arg(temp, 0, 'f', 1).arg(QChar(176)));
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
    if (topic == "weather/conditions") {
        if (object.contains("environment")) {
            QJsonObject env = object["environment"].toObject();
            m_temperature->setText(QString("%1%2").arg(env["farenheit"].toDouble(), 0, 'f', 1).arg(QChar(176)));
            m_humidity->setText(QString("%1%").arg(env["humidity"].toDouble(), 0, 'f', 1));
        }
    }
}
