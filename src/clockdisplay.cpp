// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "clockdisplay.h"

ClockDisplay::ClockDisplay(QFrame *parent) : QFrame(parent)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    if (primaryScreen) {
        // Get the screen's full geometry (resolution in pixels)
        QRect screenGeometry = primaryScreen->geometry();
        m_width = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        qDebug() << __PRETTY_FUNCTION__ << "Screen Resolution:" << m_width << "x" << screenHeight;
    }

    leftSideLayout();
    rightSideLayout();

    QHBoxLayout *parentLayout = new QHBoxLayout();
    parentLayout->addWidget(m_leftSideWidget, 60);
    parentLayout->addWidget(m_rightSideWidget, 40);
    setLayout(parentLayout);

    m_clockTimer = new QTimer();
    connect(m_clockTimer, &QTimer::timeout, this, &ClockDisplay::clockTimeout);
    m_clockTimer->setInterval(500);

    clockTimeout();

    m_tempsDb = QSqlDatabase::addDatabase("QMARIADB");
    m_tempsDb.setHostName(settings.value("dbserver").toString());
    m_tempsDb.setDatabaseName(settings.value("db").toString());
    m_tempsDb.setUserName(settings.value("dbusername").toString());
    m_tempsDb.setPassword(settings.value("dbpass").toString());

    m_weather = new Weather();
    connect(m_weather, &Weather::forecast, this, &ClockDisplay::forecastConditions);

    sunPositionUpdate();

    QTimer::singleShot(60000, m_weather, &Weather::getToday);
}

ClockDisplay::~ClockDisplay()
{
}

void ClockDisplay::leftSideLayout()
{
    m_leftSideLayout = new QGridLayout();
    m_leftSideWidget = new QWidget();

    m_primaryClock = new CustomLabel(60, m_width);
    m_primaryClock->setStyleSheet("QLabel{color:#FFFFC0}");
    m_primaryDate = new CustomLabel(20, m_width);
    m_localHighTempLabel = new CustomLabel("Today's High", 14, m_width);
    m_localHighTemp = new CustomLabel(18, m_width);
    m_localLowTemp = new CustomLabel(18, m_width);
    m_localLowTempLabel = new CustomLabel("Today's Low", 14, m_width);
    m_forecastHighTemp = new CustomLabel(18, m_width);
    m_forecastHighTempLabel = new CustomLabel("Forecast High", 14, m_width);
    m_forecastLowTemp = new CustomLabel(18, m_width);
    m_forecastLowTempLabel = new CustomLabel("Forecast Low", 14, m_width);
    m_sunrise = new CustomLabel(18, m_width);
    m_sunset = new CustomLabel(18, m_width);
    m_sunriseLabel = new CustomLabel("Sunrise", 14, m_width);
    m_sunsetLabel = new CustomLabel("Sunset", 14, m_width);

                                                    // row, col, rowspan, colspan
    m_leftSideLayout->addWidget(m_primaryClock,          0, 0, 3, 2, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_primaryDate,           3, 0, 2, 2, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localLowTempLabel,     6, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localLowTemp,          7, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localHighTempLabel,    6, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_localHighTemp,         7, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastLowTempLabel,  8, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastLowTemp,       9, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastHighTempLabel, 8, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_forecastHighTemp,      9, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunriseLabel,          10, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunrise,               11, 0, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunsetLabel,           10, 1, 1, 1, Qt::AlignCenter);
    m_leftSideLayout->addWidget(m_sunset,                11, 1, 1, 1, Qt::AlignCenter);
    m_leftSideWidget->setLayout(m_leftSideLayout);
}

void ClockDisplay::rightSideLayout()
{
    m_rightSideLayout = new QGridLayout();
    m_rightSideWidget = new QWidget();

    CustomLabel *outside = new CustomLabel("Outside", 20, m_width);
    CustomLabel *inside = new CustomLabel("Inside", 20, m_width);

    m_localTemperatureLabel = new CustomLabel("Temperature", 16, m_width);
    m_localTemperatureSymbol = new CustomLabel(QChar(176), 12, m_width, Qt::AlignLeft|Qt::AlignVCenter);
    m_localTemperature = new CustomLabel(16, m_width);
    m_localHumidityLabel = new CustomLabel("Humidity", 16, m_width);
    m_localHumidity = new CustomLabel(16, m_width);
    m_localHumiditySymbol = new CustomLabel("%", 12, m_width, Qt::AlignLeft);
    m_outdoorTemperatureLabel = new CustomLabel("Temperature", 16, m_width);
    m_outdoorTemperatureSymbol = new CustomLabel(QChar(176), 12, m_width, Qt::AlignLeft|Qt::AlignVCenter);
    m_outdoorTemperature = new CustomLabel(16, m_width);
    m_outdoorHumidtyLabel = new CustomLabel("Humidity", 16, m_width);
    m_outdoorHumiditySymbol = new CustomLabel("%", 12, m_width, Qt::AlignLeft|Qt::AlignVCenter);
    m_outdoorHumidity = new CustomLabel(16, m_width);
    m_uvIndexLabel = new CustomLabel("UV Index", 16, m_width);
    m_uvIndex = new CustomLabel(16, m_width);
    m_rainFallSymbol = new CustomLabel("in", 12, m_width);
    m_rainFallLabel = new CustomLabel("Rain Today", 16, m_width, Qt::AlignLeft|Qt::AlignVCenter);
    m_rainFall = new CustomLabel(16, m_width);


    m_rightSideLayout->addWidget(outside,                    0, 0, 1, 5, Qt::AlignCenter);
    m_rightSideLayout->addWidget(m_outdoorTemperatureLabel,  1, 0, 1, 3, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_outdoorTemperature,       1, 3, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_outdoorTemperatureSymbol, 1, 4, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_outdoorHumidtyLabel,      2, 0, 1, 3, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_outdoorHumidity,          2, 3, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_outdoorHumiditySymbol,    2, 4, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_rainFallLabel,            3, 0, 1, 3, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_rainFall,                 3, 3, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_rainFallSymbol,           3, 4, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_uvIndexLabel,             4, 0, 1, 3, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_uvIndex,                  4, 3, 1, 1, Qt::AlignRight);

    m_rightSideLayout->addWidget(inside,                     5, 0, 1, 5, Qt::AlignCenter);
    m_rightSideLayout->addWidget(m_localTemperatureLabel,    6, 0, 1, 3, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_localTemperature,         6, 3, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_localTemperatureSymbol,   6, 4, 1, 1, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_localHumidityLabel,       7, 0, 1, 3, Qt::AlignLeft);
    m_rightSideLayout->addWidget(m_localHumidity,            7, 3, 1, 1, Qt::AlignRight);
    m_rightSideLayout->addWidget(m_localHumiditySymbol,      7, 4, 1, 1, Qt::AlignLeft);
    m_rightSideWidget->setLayout(m_rightSideLayout);
}

void ClockDisplay::setLocalLowHighTemps()
{
    if (m_tempsDb.open()) {
        QSqlQuery query("SELECT * FROM limits WHERE DATE(timestamp) = CURDATE()", m_tempsDb);
        while (query.next()) {
            if (query.value(1).toString() == "HIGH") {
                QDateTime when = QDateTime::fromString(query.value(3).toString(), Qt::ISODateWithMs);
                QColor c = mapValuesNoString(query.value(2).toDouble(), 0, 100, 0, 320);
                QString span = QString("<span style='color:#%1%2%3'>").arg(c.red(), 2, 16, QChar('0')).arg(c.green(), 2, 16, QChar('0')).arg(c.blue(), 2, 16, QChar('0'));
                m_localHighTemp->setText(QString("%1%2</span>%3 <span style='font-size:10px'>(%4)</span>").arg(span).arg(query.value(2).toDouble(), 0, 'f', 1).arg(QChar(176)).arg(when.time().toString("hh:mm")));
            }
            if (query.value(1).toString() == "LOW") {
                QDateTime when = QDateTime::fromString(query.value(3).toString(), Qt::ISODateWithMs);
                QColor c = mapValuesNoString(query.value(2).toDouble(), 0, 100, 0, 320);
                QString span = QString("<span style='color:#%1%2%3'>").arg(c.red(), 2, 16, QChar('0')).arg(c.green(), 2, 16, QChar('0')).arg(c.blue(), 2, 16, QChar('0'));
                m_localLowTemp->setText(QString("%1%2</span>%3 <span style='font-size:10px'>(%4)</span>").arg(span).arg(query.value(2).toDouble(), 0, 'f', 1).arg(QChar(176)).arg(when.time().toString("hh:mm")));
            }
        }
        m_tempsDb.close();
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << "Unable to open database" << m_tempsDb.lastError().text();
    }

    QTimer::singleShot(60000, this, &ClockDisplay::setLocalLowHighTemps);
}

void ClockDisplay::showEvent(QShowEvent* e)
{
    Q_UNUSED(e)
    qDebug() << __PRETTY_FUNCTION__;
    m_clockTimer->start();
}

void ClockDisplay::hideEvent(QHideEvent* e)
{
    Q_UNUSED(e)
    qDebug() << __PRETTY_FUNCTION__;
    m_clockTimer->stop();
}

void ClockDisplay::sunPositionUpdate()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");

    SunSet sun(settings.value("lat").toDouble(), settings.value("lon").toDouble(), 0);;

    QDateTime now = QDateTime::currentDateTime();
    QDateTime tomorrow;
    int offset = now.offsetFromUtc() / 3600;
    QTime sunrise;
    QTime sunset;
    int sr;
    int ss;

    sun.setCurrentDate(now.date().year(), now.date().month(), now.date().day());
    sun.setTZOffset(offset);
    sr = sun.calcSunrise();
    ss = sun.calcSunset();
    sunrise.setHMS(sr/60, sr%60, 0);
    sunset.setHMS(ss/60, ss%60, 0);
    m_sunrise->setText(sunrise.toString("h:mm ap"));
    m_sunset->setText(sunset.toString("h:mm ap"));

    tomorrow.setTime(QTime(3, 5, 0));
    if (now.time().hour() > 2) {
        tomorrow = now.addDays(1);
        tomorrow.setTime(QTime(3, 5, 0));
        QTimer::singleShot(now.msecsTo(tomorrow), this, &ClockDisplay::sunPositionUpdate);
    }
    else {
        QDateTime later(now.date(), QTime(3, 5, 0), now.timeZone());
        QTimer::singleShot(now.msecsTo(later), this, &ClockDisplay::sunPositionUpdate);
    }

    setLocalLowHighTemps();
    m_weather->getToday();
}

void ClockDisplay::clockTimeout()
{
    QDateTime now = QDateTime::currentDateTime();

    m_primaryDate->setText(now.date().toString("ddd MMMM d, yyyy"));
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
            m_outdoorHumidity->setText(QString("%1").arg(env["humidity"].toDouble(), 0, 'f', 1));
        }
    }
    else if (topic == "weather/light") {
        if (object.contains("uv")) {
            int uv = object["uv"].toInt();
            m_uvIndex->setText(QString("%1").arg(uv));
            m_uvIndex->setStyleSheet(mapValues(uv, 0, 9, 0, 240));
        }
    }
    else if (topic == "weather/rainfall") {
        m_rainFall->setText(QString("%1").arg(object["today"].toDouble(), 0, 'f', 2));
    }
}

void ClockDisplay::forecastConditions(double high, double low)
{
    m_forecastHighTemp->setText(QString("%1%2").arg(high, 0, 'f', 0).arg(QChar(176)));
    m_forecastHighTemp->setStyleSheet(mapValues(high, 0, 100, 0, 320));
    m_forecastLowTemp->setText(QString("%1%2").arg(low, 0, 'f', 0).arg(QChar(176)));
    m_forecastLowTemp->setStyleSheet(mapValues(low, 0, 100, 0, 320));
    QTimer::singleShot(TWENTYFOUR_HOURS, m_weather, &Weather::getToday);
}

void ClockDisplay::updateLocalConditions(double temp, double humidity)
{
    m_localTemperature->setText(QString("%1").arg(temp, 0, 'f', 1));
    m_localTemperature->setStyleSheet(mapValues(temp, 0, 100, 0, 320));
    m_localHumidity->setText(QString("%1").arg(humidity, 0, 'f', 1));
}
