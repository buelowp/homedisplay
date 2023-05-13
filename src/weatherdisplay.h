// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef WEATHERDISPLAY_H
#define WEATHERDISPLAY_H

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

#include "compassrose.h"

class WeatherDisplay : public QWidget
{
    Q_OBJECT

public:
    WeatherDisplay(QWidget *parent = nullptr);
    ~WeatherDisplay();

    void updateDisplay(QString &topic, QJsonObject &object);
        
private:
    double calculateHeatIndex(double temp, double humidity);
    double calculateWindchill(double temp, int speed);
    
    QLabel *m_temperature;
    QLabel *m_humidity;
    QLabel *m_heatIndex;
    QLabel *m_windSpeed;
    QLabel *m_uvIndex;
    QLabel *m_rainToday;
    QLabel *m_rainYTD;
    QLabel *m_usvh;
    QLabel *m_pressure;
    QLabel *m_temperatureLabel;
    QLabel *m_humidityLabel;
    QLabel *m_heatIndexLabel;
    QLabel *m_windSpeedLabel;
    QLabel *m_windDirLabel;
    QLabel *m_uvIndexLabel;
    QLabel *m_rainTodayLabel;
    QLabel *m_rainYTDLabel;
    QLabel *m_pressureLabel;
    QLabel *m_usvhLabel;
    CompassRose *m_rose;
    QGridLayout *m_layout;
    int m_lastWS;
};

#endif // WEATHERDISPLAY_H
