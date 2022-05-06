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
    QLabel *m_temperature;
    QLabel *m_humidity;
    QLabel *m_heatIndex;
    QLabel *m_windSpeed;
    QLabel *m_uvIndex;
    QLabel *m_rainToday;
    QLabel *m_rainYTD;
    QLabel *m_usvh;
    CompassRose *m_rose;
    QGridLayout *m_layout;
};

#endif // WEATHERDISPLAY_H
