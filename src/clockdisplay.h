// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <QtSql/QtSql>
#include <sunset.h>

#include "environment.h"
#include "weather.h"
#include "customlabel.h"
#include "defines.h"

class ClockDisplay : public QFrame
{
    Q_OBJECT

public:
    ClockDisplay(QFrame *parent = nullptr);
    ~ClockDisplay();

    void updateDisplay(QString &topic, QJsonObject &object);

signals:
    void startDimScreen();

protected:
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

public slots:
    void clockTimeout();
    void updateLocalConditions(double temp, double humidity);
    void forecastConditions(double high, double low);
    void sunPositionUpdate();
    void setLocalLowHighTemps();

private:
    void leftSideLayout();
    void rightSideLayout();
    QString mapValues(double input, double input_start, double input_end, double output_start, double output_end);
    QColor mapValuesNoString(double input, double input_start, double input_end, double output_start, double output_end);

    Weather *m_weather;
    Environment *m_environment;

    QGridLayout *m_leftSideLayout;
    QWidget *m_leftSideWidget;
    QGridLayout *m_rightSideLayout;
    QWidget *m_rightSideWidget;

	CustomLabel *m_primaryClock;
	CustomLabel *m_primaryDate;
    CustomLabel *m_localTemperature;
    CustomLabel *m_localTemperatureLabel;
    CustomLabel *m_localTemperatureSymbol;
    CustomLabel *m_localHumidity;
    CustomLabel *m_localHumidityLabel;
    CustomLabel *m_localHumiditySymbol;
    CustomLabel *m_outdoorTemperature;
    CustomLabel *m_outdoorTemperatureSymbol;
    CustomLabel *m_outdoorTemperatureLabel;
    CustomLabel *m_outdoorHumidity;
    CustomLabel *m_outdoorHumidtyLabel;
    CustomLabel *m_outdoorHumiditySymbol;
    CustomLabel *m_localHighTemp;
    CustomLabel *m_localHighTempLabel;
    CustomLabel *m_localLowTemp;
    CustomLabel *m_localLowTempLabel;
    CustomLabel *m_forecastHighTemp;
    CustomLabel *m_forecastHighTempLabel;
    CustomLabel *m_forecastLowTemp;
    CustomLabel *m_forecastLowTempLabel;
    CustomLabel *m_uvIndexLabel;
    CustomLabel *m_uvIndex;
    CustomLabel *m_sunrise;
    CustomLabel *m_sunset;
    CustomLabel *m_sunriseLabel;
    CustomLabel *m_sunsetLabel;
    CustomLabel *m_rainFallLabel;
    CustomLabel *m_rainFallSymbol;
    CustomLabel *m_rainFall;
    QTimer *m_clockTimer;
    QTimer *m_weatherUpdateTimer;
    QSqlDatabase m_tempsDb;
    int m_width;
};

#endif // CLOCKDISPLAY_H
