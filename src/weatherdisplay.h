// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef WEATHERDISPLAY_H
#define WEATHERDISPLAY_H

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

#include "compassrose.h"
#include "customlabel.h"

class WeatherDisplay : public QWidget
{
    Q_OBJECT

public:
    WeatherDisplay(QWidget *parent = nullptr);
    ~WeatherDisplay();

    void updateDisplay(QString &topic, QJsonObject &object);

public slots:
    void precip(double p);
        
private:
    double calculateHeatIndex(double temp, double humidity);
    double calculateWindchill(double temp, double speed);
    
    CustomLabel *m_temperature;
    CustomLabel *m_humidity;
    CustomLabel *m_heatIndex;
    CustomLabel *m_windSpeed;
    CustomLabel *m_uvIndex;
    CustomLabel *m_rainToday;
    CustomLabel *m_rainYTD;
    CustomLabel *m_usvh;
    CustomLabel *m_pressure;
    CustomLabel *m_precip;
    CustomLabel *m_temperatureLabel;
    CustomLabel *m_humidityLabel;
    CustomLabel *m_heatIndexLabel;
    CustomLabel *m_windSpeedLabel;
    CustomLabel *m_windDirLabel;
    CustomLabel *m_uvIndexLabel;
    CustomLabel *m_rainTodayLabel;
    CustomLabel *m_rainYTDLabel;
    CustomLabel *m_pressureLabel;
    CustomLabel *m_usvhLabel;
    CustomLabel *m_rainChanceLabel;
    CompassRose *m_rose;
    QGridLayout *m_layout;
    double m_lastWS;
    double m_lastTemp;
    int m_width;
};

#endif // WEATHERDISPLAY_H
