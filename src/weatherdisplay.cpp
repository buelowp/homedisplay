// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "weatherdisplay.h"

WeatherDisplay::WeatherDisplay(QWidget *parent) : QWidget(parent)
{
    m_lastWS = 0;
    m_lastTemp = -100.0;

    QFont l("Roboto-Regular", 28);
    QFont p("Roboto-Regular", 12);

    setWindowState(Qt::WindowFullScreen);

    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    if (primaryScreen) {
        // Get the screen's full geometry (resolution in pixels)
        QRect screenGeometry = primaryScreen->geometry();
        m_width = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        qDebug() << __PRETTY_FUNCTION__ << "Screen Resolution:" << m_width << "x" << screenHeight;
    }

    m_temperature = new CustomLabel(28, m_width);
    m_heatIndex = new CustomLabel(28, m_width);
    m_humidity = new CustomLabel(28, m_width);
    m_rose = new CompassRose();
    m_uvIndex = new CustomLabel(28, m_width);
    m_windSpeed = new CustomLabel(28, m_width);
    m_usvh = new CustomLabel(28, m_width);
    m_rainToday = new CustomLabel(28, m_width);
    m_rainYTD = new CustomLabel(28, m_width);
    m_pressure = new CustomLabel(28, m_width);
    m_precip = new CustomLabel(28, m_width);

    m_temperatureLabel = new CustomLabel("Temperature", 12, m_width);
    m_heatIndexLabel = new CustomLabel("Heat Index", 12, m_width);
    m_humidityLabel = new CustomLabel("Humidity", 12, m_width);
    m_uvIndexLabel = new CustomLabel("UV Index", 12, m_width);
    m_windSpeedLabel = new CustomLabel("Wind Speed", 12, m_width);
    m_windDirLabel = new CustomLabel("Wind Direction", 12, m_width);
    m_usvhLabel = new CustomLabel("Radiation", 12, m_width);
    m_rainTodayLabel = new CustomLabel("Rainfall Today", 12, m_width);
    m_rainYTDLabel = new CustomLabel("Rainfall YTD", 12, m_width);
    m_pressureLabel = new CustomLabel("Barometer", 12, m_width);
    m_rainChanceLabel = new CustomLabel("% Precip", 12, m_width);

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
    m_layout->addWidget(m_rainChanceLabel, 6, 2, 1, 1);
    m_layout->addWidget(m_precip, 6, 2, 1, 1);
    
    setLayout(m_layout);
}

WeatherDisplay::~WeatherDisplay()
{
}

void WeatherDisplay::precip(double p)
{
    m_precip->setText(QString("%1%").arg(p, 0, 'f', 0));
}

double WeatherDisplay::calculateHeatIndex(double T, double R)
{

    // Formula constants
    double c1 = -42.379, c2 = 2.04901523, c3 = 10.14333127,
    c4 = -0.22475541, c5 = -0.00683783, c6 = -0.05481717,
    c7 = 0.00122874, c8 = 0.00085282, c9 = -0.00000199;

    double HI = c1 + (c2 * T) + (c3 * R) + (c4 * T * R) +
    (c5 * T * T) + (c6 * R * R) + (c7 * T * T * R) +
    (c8 * T * R * R) + (c9 * T * T * R * R);

    // Adjustments for specific conditions
    if (R < 13.0 && T >= 80.0 && T <= 112.0) {
        HI -= ((13.0 - R) / 4.0) * sqrt((17.0 - fabs(T - 95.0)) / 17.0);
    } else if (R > 85.0 && T >= 80.0 && T <= 87.0) {
        HI += ((R - 85.0) / 10.0) * ((87.0 - T) / 5.0);
    }

    return HI;
}

double WeatherDisplay::calculateWindchill(double temp, double speed)
{
    if (temp > 40.0 || speed <= 3.0) {
        return temp;
    }
    else {
        // Use the NWS wind chill formula
        double wind_chill = 35.74 + (0.6215 * temp) - (35.75 * pow(speed, 0.16)) + (0.4275 * temp * pow(speed, 0.16));
        return wind_chill;
    }
}

/*
 * {"timestamp": "06/05/2022 18:05:17", "environment": {"celcius": 11.426718547341117, "farenheit": 52.56809338521401, "humidity": 68.05981536583505}, "station": {"boxtemp": 63.1625, "cputemp": 90.70700000000001, "uptime": 95714.51}, "radiation": {"duration": 91244.24, "cpm": 2.6, "uSvh": 0.049, "uSvhError": 0.007}}
 */
void WeatherDisplay::updateDisplay(QString &topic, QJsonObject &object)
{
    if (topic == "weather/rainfall") {
        if (object.contains("ytd")) {
            m_rainYTD->setText(QString("%1\"").arg(object["ytd"].toDouble(), 0, 'f', 2));
            m_rainToday->setText(QString("%1\"").arg(object["today"].toDouble(),0, 'f', 2));
        }
    }
    else if (topic == "weather/conditions") {
        if (object.contains("environment")) {
            QJsonObject env = object["environment"].toObject();
            QJsonObject radiation = object["radiation"].toObject();
            m_lastTemp = env["farenheit"].toDouble();
            double celsius = env["celsius"].toDouble();
            double humidity = env["humidity"].toDouble();
            m_temperature->setText(QString("%1%2").arg(m_lastTemp, 0, 'f', 1).arg(QChar(176)));
            m_humidity->setText(QString("%1%").arg(humidity, 0, 'f', 1));
            if (m_lastTemp >= 80.0) {
                m_heatIndexLabel->setText("Heat Index");
                m_heatIndex->setText(QString("%1%2").arg(calculateHeatIndex(celsius, humidity), 0, 'f', 1).arg(QChar(176)));
            }
            else {
                m_heatIndexLabel->setText("");
                m_heatIndex->setText(QString("%1%2").arg(m_lastTemp).arg(QChar(176)));
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
            m_lastWS = object["speed"].toDouble();
            m_windSpeed->setText(QString("%1 mph").arg(m_lastWS, 0, 'f', 1));
            m_rose->setAngle(object["direction"].toInt());
            if (m_lastTemp != -100) {
                m_heatIndexLabel->setText("Wind Chill");
                m_heatIndex->setText(QString("%1%2").arg(calculateWindchill(m_lastTemp, m_lastWS), 0, 'f', 1).arg(QChar(176)));
            }
        }
    }
    else if (topic == "weather/barometer") {
        if (object.contains("pressure")) {
            double pressure = object["pressure"].toDouble();
            m_pressure->setText(QString("%1 inHg").arg(pressure, 0, 'f', 2));
        }
    }
}
