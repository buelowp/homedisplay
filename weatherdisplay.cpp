// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "weatherdisplay.h"

WeatherDisplay::WeatherDisplay(QWidget *parent) : QWidget(parent)
{
    m_temperature = new QLabel();
    m_heatIndex = new QLabel();
    m_humidity = new QLabel();
    m_rose = new CompassRose();
    m_uvIndex = new QLabel();
    m_windSpeed = new QLabel();
    m_usvh = new QLabel();
    m_rainToday = new QLabel();
    m_rainYTD = new QLabel();

    m_layout = new QGridLayout();
    m_layout->addWidget(m_temperature, 0, 0, 1, 1);
    m_layout->addWidget(m_humidity, 0, 1, 1, 1);
    m_layout->addWidget(m_heatIndex, 0, 2, 1, 1);
    m_layout->addWidget(m_windSpeed, 1, 0, 1, 1);
    m_layout->addWidget(m_rose, 1, 1, 2, 2);
    m_layout->addWidget(m_rainToday, 2, 0, 1, 1);
    m_layout->addWidget(m_rainYTD, 2, 1, 1, 1);
    m_layout->addWidget(m_usvh, 3, 0, 1, 1);
    m_layout->addWidget(m_uvIndex, 3, 1, 1, 1);
    
    setLayout(m_layout);
}

WeatherDisplay::~WeatherDisplay()
{
}

/*
 * {"timestamp": "06/05/2022 18:05:17", "environment": {"celcius": 11.426718547341117, "farenheit": 52.56809338521401, "humidity": 68.05981536583505}, "station": {"boxtemp": 63.1625, "cputemp": 90.70700000000001, "uptime": 95714.51}, "radiation": {"duration": 91244.24, "cpm": 2.6, "uSvh": 0.049, "uSvhError": 0.007}}
 */
void WeatherDisplay::updateDisplay(QString &topic, QJsonObject &object)
{
    if (topic == "weather/rainfall") {
        if (object.contains("ytd")) {
            m_rainYTD->setText(QString("%1").arg(object["ytd"].toDouble()));
            m_rainToday->setText(QString("%1").arg(object["today"].toDouble()));
        }
    }
    
    if (topic == "weather/conditions") {
        if (object.contains("environment")) {
            QJsonObject env = object["environment"].toObject();
            QJsonObject radiation = object["radiation"].toObject();
            m_temperature->setText(QString("%1").arg(env["farenheit"].toDouble()));
            m_humidity->setText(QString("%1%").arg(env["humidity"].toDouble()));
            m_usvh->setText(QString("%1 usvh").arg(radiation["uSvh"].toDouble()));
        }
    }
    
    if (topic == "weather/light") {
    }
    
    if (topic == "weather/wind") {
        if (object.contains("speed")) {
            m_windSpeed->setText(QString("%1 mph").arg(object["speed"].toString()));
            m_rose->setAngle(object["direction"].toInt());
        }
    }
}
