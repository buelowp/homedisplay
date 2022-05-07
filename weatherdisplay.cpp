// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "weatherdisplay.h"

WeatherDisplay::WeatherDisplay(QWidget *parent) : QWidget(parent)
{
    QFont l("Roboto-Regular", 28);

    m_temperature = new QLabel();
    m_temperature->setFont(l);
    m_temperature->setScaledContents(true);
    m_temperature->setAlignment(Qt::AlignCenter);
    m_heatIndex = new QLabel();
    m_heatIndex->setFont(l);
    m_heatIndex->setScaledContents(true);
    m_heatIndex->setAlignment(Qt::AlignCenter);
    m_humidity = new QLabel();
    m_humidity->setFont(l);
    m_humidity->setScaledContents(true);
    m_humidity->setAlignment(Qt::AlignCenter);
    m_rose = new CompassRose();
    m_uvIndex = new QLabel();
    m_uvIndex->setFont(l);
    m_uvIndex->setScaledContents(true);
    m_uvIndex->setAlignment(Qt::AlignCenter);
    m_windSpeed = new QLabel();
    m_windSpeed->setFont(l);
    m_windSpeed->setScaledContents(true);
    m_windSpeed->setAlignment(Qt::AlignCenter);
    m_usvh = new QLabel();
    m_usvh->setFont(l);
    m_usvh->setScaledContents(true);
    m_usvh->setAlignment(Qt::AlignCenter);
    m_rainToday = new QLabel();
    m_rainToday->setFont(l);
    m_rainToday->setScaledContents(true);
    m_rainToday->setAlignment(Qt::AlignCenter);
    m_rainYTD = new QLabel();
    m_rainYTD->setFont(l);
    m_rainYTD->setScaledContents(true);
    m_rainYTD->setAlignment(Qt::AlignCenter);

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

double WeatherDisplay::calculateHeatIndex(double temp, double humidity)
{
    double tdpfc =  (temp - (14.55 + 0.114 * temp) * (1 - (0.01 * humidity)) - pow(((2.5 + 0.007 * temp) * (1 - (0.01 * humidity))),3) - (15.9 + 0.117 * temp) * pow((1 - (0.01 * humidity)), 14));
    double heatIndex = tdpfc * 1.8 + 32;
    return heatIndex;
}

double WeatherDisplay::calculateWindchill(double temp, int speed)
{
    double w = 33-((10*sqrt(speed)-speed+10.5)*(33-temp))/23.1;
    return w;
}

/*
 * {"timestamp": "06/05/2022 18:05:17", "environment": {"celcius": 11.426718547341117, "farenheit": 52.56809338521401, "humidity": 68.05981536583505}, "station": {"boxtemp": 63.1625, "cputemp": 90.70700000000001, "uptime": 95714.51}, "radiation": {"duration": 91244.24, "cpm": 2.6, "uSvh": 0.049, "uSvhError": 0.007}}
 */
void WeatherDisplay::updateDisplay(QString &topic, QJsonObject &object)
{
    qDebug() << __PRETTY_FUNCTION__ << ": " << topic;
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
            double farenheit = env["farenheit"].toDouble();
            double celsius = env["celsius"].toDouble();
            double humidity = env["humidity"].toDouble();
            m_temperature->setText(QString("%1%2").arg(farenheit, 0, 'f', 1).arg(QChar(176)));
            m_humidity->setText(QString("%1%").arg(humidity, 0, 'f', 1));
            m_heatIndex->setText(QString("%1%2").arg(calculateHeatIndex(celsius, humidity), 0, 'f', 1).arg(QChar(176)));
            m_usvh->setText(QString("%1 usvh").arg(radiation["uSvh"].toDouble(), 0, 'f', 3));
        }
    }
    
    if (topic == "weather/light") {
        if (object.contains("uv")) {
            m_uvIndex->setText(QString("%1").arg(object["uv"].toInt()));
        }
    }
    
    if (topic == "weather/wind") {
        if (object.contains("speed")) {
            m_windSpeed->setText(QString("%1 mph").arg(object["speed"].toInt()));
            m_rose->setAngle(object["direction"].toInt());
        }
    }
}
