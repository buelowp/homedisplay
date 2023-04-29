// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "clockdisplay.h"

ClockDisplay::ClockDisplay(QFrame *parent) : QFrame(parent)
{
    leftSideLayout();
    rightSideLayout();

    QHBoxLayout *parentLayout = new QHBoxLayout();
    parentLayout->addWidget(m_leftSideWidget);
    parentLayout->addWidget(m_rightSideWidget);
    setLayout(parentLayout);

    m_clockTimer = new QTimer();
    connect(m_clockTimer, &QTimer::timeout, this, &ClockDisplay::clockTimeout);
    m_clockTimer->setInterval(500);
    m_clockTimer->start();

    clockTimeout();

    m_weather = new Weather();
    connect(m_weather, &Weather::forecast, this, &ClockDisplay::forecastConditions);
    m_weather->getToday();
}

ClockDisplay::~ClockDisplay()
{
}

void ClockDisplay::leftSideLayout()
{
    m_leftSideLayout = new QGridLayout();
    m_leftSideWidget = new QWidget();

    QFont clock("Roboto-Regular", 60);
    QFont date("Roboto-Regular", 32);

    m_primaryClock = new CustomLabel(60);
    m_primaryDate = new CustomLabel(36);
    m_localHighTempLabel = new CustomLabel("Today's High", 26);
    m_localHighTemp = new CustomLabel(32);
    m_localLowTemp = new CustomLabel(32);
    m_localLowTempLabel = new CustomLabel("Today's Low", 26);
    m_forecastHighTemp = new CustomLabel(32);
    m_forecastHighTempLabel = new CustomLabel("Forecast High", 26);
    m_forecastLowTemp = new CustomLabel(32);
    m_forecastLowTempLabel = new CustomLabel("Forecast Low", 26);
    m_sunrise = new CustomLabel(32);
    m_sunset = new CustomLabel(32);
    m_sunriseLabel = new CustomLabel("Sunrise", 26);
    m_sunsetLabel = new CustomLabel("Sunset", 26);

                                                    // row, col, rowspan, colspan
    m_leftSideLayout->addWidget(m_primaryClock,          0, 0, 2, 2, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_primaryDate,           2, 0, 1, 2, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localLowTempLabel,     3, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localLowTemp,          4, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localHighTempLabel,    3, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localHighTemp,         4, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastLowTempLabel,  5, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastLowTemp,       6, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastHighTempLabel, 5, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastHighTemp,      6, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunriseLabel,          7, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunrise,               8, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunsetLabel,           7, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunset,                8, 1, 1, 1, Qt::AlignCenter);
    m_leftSideWidget->setLayout(m_leftSideLayout);
}

void ClockDisplay::rightSideLayout()
{
    m_rightSideLayout = new QGridLayout();
    m_rightSideWidget = new QWidget();

    CustomLabel *outside = new CustomLabel("Outside", 36);
    CustomLabel *inside = new CustomLabel("Inside", 36);

    m_localTemperatureLabel = new CustomLabel("Temperature", 32);
    m_localTemperatureSymbol = new CustomLabel(QChar(176), 18, Qt::AlignLeft|Qt::AlignBottom);
    m_localTemperature = new CustomLabel(32);
    m_localHumidityLabel = new CustomLabel("Humidity", 32);
    m_localHumidity = new CustomLabel(32);
    m_localHumiditySymbol = new CustomLabel("%", 18, Qt::AlignLeft|Qt::AlignBottom);
    m_outdoorTemperatureLabel = new CustomLabel("Temperature", 32);
    m_outdoorTemperatureSymbol = new CustomLabel(QChar(176), 18, Qt::AlignLeft|Qt::AlignBottom);
    m_outdoorTemperature = new CustomLabel(32);
    m_outdoorHumidtyLabel = new CustomLabel("Humidity", 32);
    m_outdoorHumiditySymbol = new CustomLabel("%", 18, Qt::AlignLeft|Qt::AlignBottom);
    m_outdoorHumidity = new CustomLabel(32);
    m_uvIndexLabel = new CustomLabel("UV Index", 32);
    m_uvIndex = new CustomLabel(32);


    m_rightSideLayout->addWidget(outside,                    0, 0, 1, 3, Qt::AlignCenter);
    m_rightSideLayout->addWidget(m_outdoorTemperatureLabel,  1, 0, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_outdoorTemperature,       1, 1, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_outdoorTemperatureSymbol, 1, 2, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_outdoorHumidtyLabel,      2, 0, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_outdoorHumidity,          2, 1, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_outdoorHumiditySymbol,    2, 2, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_uvIndexLabel,             3, 0, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_uvIndex,                  3, 1, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(inside,                     4, 0, 1, 3, Qt::AlignCenter);
    m_rightSideLayout->addWidget(m_localTemperatureLabel,    5, 0, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_localTemperature,         5, 1, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_localTemperatureSymbol,   5, 2, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_localHumidityLabel,       6, 0, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_localHumidity,            6, 1, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_localHumiditySymbol,      6, 2, 1, 1, Qt::AlignLeft);
    m_rightSideWidget->setLayout(m_rightSideLayout);
}

void ClockDisplay::showEvent(QShowEvent* e)
{
}

void ClockDisplay::clockTimeout()
{
    QDateTime now = QDateTime::currentDateTime();

    m_primaryDate->setText(now.date().toString("dddd MMMM d, yyyy"));
    m_primaryClock->setText(now.time().toString("h:mm A"));
}

QString ClockDisplay::mapValues(double input, double input_start, double input_end, double output_start, double output_end)
{
    QColor c;

    double slope = 1.0 * (output_end - output_start) / (input_end - input_start);
    double rval = output_start + slope * (input - input_start);
    double t = floor(rval + .5);
    double converted = output_end - t;

    if (input >= input_end)
        c.setHsv(output_start, 255, 255);
    else if (input <= input_start)
        c.setHsv(output_end, 255, 255);
    else {
        c.setHsv(converted, 255, 255);
    }
    return QString("QLabel{color:#%1%2%3}").arg(c.red(), 2, 16, QChar('0')).arg(c.green(), 2, 16, QChar('0')).arg(c.blue(), 2, 16, QChar('0'));
}

QColor ClockDisplay::mapValuesNoString(double input, double input_start, double input_end, double output_start, double output_end)
{
    QColor c;

    double slope = 1.0 * (output_end - output_start) / (input_end - input_start);
    double rval = output_start + slope * (input - input_start);
    double t = floor(rval + .5);
    double converted = output_end - t;

    c.setHsv(255, 255, 255);
    if (input >= input_end)
        c.setHsv(output_start, 255, 255);
    else if (input <= input_start)
        c.setHsv(output_end, 255, 255);
    else {
        c.setHsv(converted, 255, 255);
    }
    return c;
}

void ClockDisplay::updateDisplay(QString &topic, QJsonObject &object)
{
    if (topic == "weather/conditions") {
        if (object.contains("environment")) {
            QJsonObject env = object["environment"].toObject();
            m_outdoorTemperature->setText(QString("%1").arg(env["farenheit"].toDouble(), 0, 'f', 1));
            m_outdoorTemperature->setStyleSheet(mapValues(env["farenheit"].toDouble(), 0, 100, 0, 320));
            m_outdoorHumidity->setText(QString("%1%").arg(env["humidity"].toDouble(), 0, 'f', 1));
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

void ClockDisplay::forecastConditions(double high, double low)
{
    m_forecastHighTemp->setText(QString("%1%2").arg(high, 0, 'f', 0).arg(QChar(176)));
    m_forecastLowTemp->setText(QString("%1%2").arg(low, 0, 'f', 0).arg(QChar(176)));
}

void ClockDisplay::updateLocalConditions(double temp, double humidity)
{
    m_localTemperature->setText(QString("%1").arg(temp, 0, 'f', 1));
    m_localTemperature->setStyleSheet(mapValues(temp, 0, 100, 0, 320));
    m_localHumidity->setText(QString("%1").arg(humidity, 0, 'f', 1));
}
