// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "weatherdisplay.h"

WeatherDisplay::WeatherDisplay(QWidget *parent) : QWidget(parent)
{
    m_lastWS = 0;
    
    QFont l("Roboto-Regular", 28);
    QFont p("Roboto-Regular", 12);

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
    m_pressure = new QLabel();
    m_pressure->setFont(l);
    m_pressure->setScaledContents(true);
    m_pressure->setAlignment(Qt::AlignCenter);

    m_temperatureLabel = new QLabel("Temperature");
    m_temperatureLabel->setFont(p);
    m_temperatureLabel->setScaledContents(true);
    m_temperatureLabel->setAlignment(Qt::AlignCenter);
    m_heatIndexLabel = new QLabel("Heat Index");
    m_heatIndexLabel->setFont(p);
    m_heatIndexLabel->setScaledContents(true);
    m_heatIndexLabel->setAlignment(Qt::AlignCenter);
    m_humidityLabel = new QLabel("Humidity");
    m_humidityLabel->setFont(p);
    m_humidityLabel->setScaledContents(true);
    m_humidityLabel->setAlignment(Qt::AlignCenter);
    m_uvIndexLabel = new QLabel("UV Index");
    m_uvIndexLabel->setFont(p);
    m_uvIndexLabel->setScaledContents(true);
    m_uvIndexLabel->setAlignment(Qt::AlignCenter);
    m_windSpeedLabel = new QLabel("Wind Speed");
    m_windSpeedLabel->setFont(p);
    m_windSpeedLabel->setScaledContents(true);
    m_windSpeedLabel->setAlignment(Qt::AlignCenter);
    m_windDirLabel = new QLabel("Wind Direction");
    m_windDirLabel->setFont(p);
    m_windDirLabel->setScaledContents(true);
    m_windDirLabel->setAlignment(Qt::AlignCenter);
    m_usvhLabel = new QLabel("Radiation");
    m_usvhLabel->setFont(p);
    m_usvhLabel->setScaledContents(true);
    m_usvhLabel->setAlignment(Qt::AlignCenter);
    m_rainTodayLabel = new QLabel("Rainfall Today");
    m_rainTodayLabel->setFont(p);
    m_rainTodayLabel->setScaledContents(true);
    m_rainTodayLabel->setAlignment(Qt::AlignCenter);
    m_rainYTDLabel = new QLabel("Rainfall YTD");
    m_rainYTDLabel->setFont(p);
    m_rainYTDLabel->setScaledContents(true);
    m_rainYTDLabel->setAlignment(Qt::AlignCenter);
    m_pressureLabel = new QLabel("Atmospheric Pressure");
    m_pressureLabel->setFont(p);
    m_pressureLabel->setScaledContents(true);
    m_pressureLabel->setAlignment(Qt::AlignCenter);


    m_layout = new QGridLayout();
    m_layout->addWidget(m_temperatureLabel, 0, 0, 1, 1);
    m_layout->addWidget(m_temperature, 1, 0, 1, 1);
    m_layout->addWidget(m_humidityLabel, 0, 1, 1, 1);
    m_layout->addWidget(m_humidity, 1, 1, 1, 1);
    m_layout->addWidget(m_heatIndexLabel, 0, 2, 1, 1);
    m_layout->addWidget(m_heatIndex, 1, 2, 1, 1);
    m_layout->addWidget(m_pressureLabel, 2, 0, 1, 1);
    m_layout->addWidget(m_windSpeedLabel, 2, 1, 1, 1);
    m_layout->addWidget(m_pressure, 3, 0, 1, 1);
    m_layout->addWidget(m_windSpeed, 3, 1, 1, 1);
    m_layout->addWidget(m_windDirLabel, 2, 2, 1, 1);
    m_layout->addWidget(m_rose, 3, 2, 3, 1);
    m_layout->addWidget(m_rainTodayLabel, 4, 0, 1, 1);
    m_layout->addWidget(m_rainToday, 5, 0, 1, 1);
    m_layout->addWidget(m_rainYTDLabel, 4, 1, 1, 1);
    m_layout->addWidget(m_rainYTD, 5, 1, 1, 1);
    m_layout->addWidget(m_usvhLabel, 6, 0, 1, 1);
    m_layout->addWidget(m_usvh, 7, 0, 1, 1);
    m_layout->addWidget(m_uvIndexLabel, 6, 1, 1, 1);
    m_layout->addWidget(m_uvIndex, 7, 1, 1, 1);
    
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
    if (topic == "weather/rainfall") {
        if (object.contains("ytd")) {
            m_rainYTD->setText(QString("%1").arg(object["ytd"].toDouble()));
            m_rainToday->setText(QString("%1").arg(object["today"].toDouble()));
        }
    }
    else if (topic == "weather/conditions") {
        if (object.contains("environment")) {
            QJsonObject env = object["environment"].toObject();
            QJsonObject radiation = object["radiation"].toObject();
            double farenheit = env["farenheit"].toDouble();
            double celsius = env["celsius"].toDouble();
            double humidity = env["humidity"].toDouble();
            m_temperature->setText(QString("%1%2").arg(farenheit, 0, 'f', 1).arg(QChar(176)));
            m_humidity->setText(QString("%1%").arg(humidity, 0, 'f', 1));
            if (farenheit >= 50.0) {
                m_heatIndexLabel->setText("Heat Index");
                m_heatIndex->setText(QString("%1%2").arg(calculateHeatIndex(celsius, humidity), 0, 'f', 1).arg(QChar(176)));
            }
            else {
                m_heatIndexLabel->setText("Wind Chill");
                m_heatIndex->setText(QString("%1").arg(calculateWindchill(farenheit, m_lastWS), 0, 'f', 1));
            }
            m_usvh->setText(QString("%1 usv/h").arg(radiation["uSvh"].toDouble(), 0, 'f', 3));
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
                    m_uvIndex->setText(QString("<span style=\"color:green;\">%1</span>").arg(uv));
                    break;
                case 4:
                case 5:
                case 6:
                    m_uvIndex->setText(QString("<span style=\"color:yellow;\">%1</span>").arg(uv));
                    break;
                default:
                    m_uvIndex->setText(QString("<span style=\"color:red;\">%1</span>").arg(uv));
                    break;
            }
        }
    }
    else if (topic == "weather/wind") {
        if (object.contains("speed")) {
            m_lastWS = object["speed"].toInt();
            m_windSpeed->setText(QString("%1 mph").arg(m_lastWS));
            m_rose->setAngle(object["direction"].toInt());
        }
    }
    else if (topic == "weather/pressure") {
        qDebug() << __PRETTY_FUNCTION__ << ":" << object;
        if (object.contains("pressure")) {
            double pressure = object["pressure"].toDouble();
            qDebug() << __PRETTY_FUNCTION__ << ":" << pressure;
            m_pressure->setText(QString("%1 inHg").arg(pressure, 0, 'f', 2));
        }
    }
}
