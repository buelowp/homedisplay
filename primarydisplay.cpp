/*
    This file is part of MythClock.

    MythClock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MythClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MythClock.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "primarydisplay.h"

PrimaryDisplay::PrimaryDisplay() : QMainWindow() 
{
    m_setHidden = false;
    
    QPalette pal(QColor(0,0,0));
    setBackgroundRole(QPalette::Window);
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    m_primaryLayoutWidget = new QWidget();
    m_primaryLayout = new QGridLayout();
    m_primaryClock = new QLabel();
    m_primaryClock->setScaledContents(true);
    m_primaryClock->setAlignment(Qt::AlignCenter);
    m_primaryDate = new QLabel();
    m_primaryDate->setAlignment(Qt::AlignCenter);
    m_primaryDate->setScaledContents(true);
    m_temperature = new QLabel();
    m_temperature->setScaledContents(true);
    m_humidity = new QLabel();
    m_humidity->setScaledContents(true);
    m_temperature->setAlignment(Qt::AlignCenter);
    m_humidity->setAlignment(Qt::AlignCenter);
    m_rainLabel = new QLabel();
    m_rainLabel->setAlignment(Qt::AlignCenter);
    m_rainLabel->setScaledContents(true);
    m_uvIndex = new QLabel();
    m_uvIndex->setAlignment(Qt::AlignCenter);
    m_uvIndex->setScaledContents(true);

    m_primaryLayout->addWidget(m_primaryClock, 0, 0, 1, 4);
    m_primaryLayout->addWidget(m_temperature, 2, 0, 1, 2);
    m_primaryLayout->addWidget(m_humidity, 2, 2, 1, 2);
    m_primaryLayout->addWidget(m_rainLabel, 3, 0, 1, 2);
    m_primaryLayout->addWidget(m_uvIndex, 3, 2, 1, 2);
    m_primaryLayout->addWidget(m_primaryDate, 4, 0, 1, 4);
    m_primaryLayoutWidget->setLayout(m_primaryLayout);    
    
    m_nyeLayoutWidget = new QWidget();
    m_nyeLayout = new QHBoxLayout(m_nyeLayoutWidget);
    m_lbCountdown = new QLabel(m_primaryLayoutWidget);
    m_lbCountdown->setScaledContents(true);
    m_nyeLayout->addWidget(m_lbCountdown);
    
    QFont c("Roboto-Regular", 36);
    QFont l("Roboto-Regular", 28);
    QFont p("Roboto-Regular", 100);
    QFont d("Roboto-Regular", 32);

    m_primaryClock->setFont(p);
    m_primaryDate->setFont(d);
    m_temperature->setFont(c);
    m_humidity->setFont(c);
    m_rainLabel->setFont(l);
    m_uvIndex->setFont(l);

    m_weatherWidget = new WeatherDisplay();
    m_sonosWidget = new SonosDisplay();
    
    m_blankLayoutWidget = new QWidget();

    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->addWidget(m_primaryLayoutWidget);
    m_stackedWidget->addWidget(m_sonosWidget);
    m_stackedWidget->addWidget(m_nyeLayoutWidget);
    m_stackedWidget->addWidget(m_blankLayoutWidget);
    m_stackedWidget->addWidget(m_weatherWidget);
        
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
    m_clockTimer->setInterval(50);
    m_clockTimer->start();
    
    m_endWeatherScreen = new QTimer(this);
    connect(m_endWeatherScreen, &QTimer::timeout, this, &PrimaryDisplay::endWeatherScreen);

    m_startBlankScreen = new QTimer(this);
    m_endBlankScreen = new QTimer(this);
    setupBlankScreenTimers();
  
    setupMqttSubscriber();
        
    QState *primary = new QState();
    QState *metadata = new QState();
    QState *nye = new QState();
    QState *blank = new QState();
    QState *weather = new QState();

    nye->addTransition(this, SIGNAL(stopNYE()), primary);
    primary->addTransition(m_sonosWidget, SIGNAL(startSonos()), metadata);
    primary->addTransition(this, SIGNAL(startNYE()), nye);
    primary->addTransition(m_startBlankScreen, SIGNAL(timeout()), blank);
    blank->addTransition(m_endBlankScreen, SIGNAL(timeout()), primary);
    primary->addTransition(this, SIGNAL(startWeather()), weather);
    metadata->addTransition(this, SIGNAL(startNYE()), nye);
    metadata->addTransition(m_sonosWidget, SIGNAL(endSonos()), primary);
    weather->addTransition(this, SIGNAL(hideWeatherScreen()), primary);
    
    connect(metadata, SIGNAL(entered()), this, SLOT(showMetadataScreen()));
//    connect(metadata, SIGNAL(exited()), this, SLOT(endMetadataScreen()));
    connect(primary, SIGNAL(entered()), this, SLOT(showPrimaryScreen()));
    connect(nye, SIGNAL(entered()), this, SLOT(showNYEScreen()));
    connect(blank, SIGNAL(entered()), this, SLOT(showBlankScreen()));
    connect(blank, SIGNAL(exited()), this, SLOT(endBlankScreen()));
    connect(weather, SIGNAL(entered()), this, SLOT(showWeatherScreen()));

    m_states.addState(primary);
    m_states.addState(metadata);
    m_states.addState(nye);
    m_states.addState(blank);
    m_states.addState(weather);
    m_states.setInitialState(primary);

    setNYETimeout();

    enableBacklight(true);
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    setCentralWidget(m_stackedWidget);
    m_states.start();
    m_sonosWidget->go();
}

PrimaryDisplay::~PrimaryDisplay() 
{
}

void PrimaryDisplay::setupBlankScreenTimers()
{
    QDateTime now = QDateTime::currentDateTime();
    int interval;
    
    if (now.time().hour() >= 1 && now.time().hour() < 5) {
        interval = 0;
    }
    else if (now.time().hour() == 0) {
        QTime end(1,0,0);
        interval = now.time().msecsTo(end);
    }
    else {
        QDateTime tomorrow = QDateTime::currentDateTime();
        tomorrow = tomorrow.addDays(1);
        tomorrow.setTime(QTime(1,0,0));
        interval = now.msecsTo(tomorrow);
    }
    qDebug() << __PRETTY_FUNCTION__ << ": Blanking screen in" << interval / 1000 << "seconds";
    m_startBlankScreen->setInterval(interval);        
    m_startBlankScreen->setSingleShot(true);
    m_startBlankScreen->start();
}

bool PrimaryDisplay::event(QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        qDebug() << __PRETTY_FUNCTION__ << ": Starting weather widget";
        emit startWeather();
        return true;
    }
    
    return QMainWindow::event(event);
}

void PrimaryDisplay::setupMqttSubscriber()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    QString hostname = settings.value("mqttserver").toString();
    QHostInfo lookup = QHostInfo::fromName(hostname);
    QList<QHostAddress> addresses = lookup.addresses();
    
    if (addresses.size() > 0) {
        m_mqttClient = new QMqttSubscriber(addresses.at(0), settings.value("mqttport").toInt(), this);
        qDebug() << __PRETTY_FUNCTION__ << ": setting host address to" << addresses.at(0);
    }
    else {
        m_mqttClient = new QMqttSubscriber(QHostAddress::LocalHost, settings.value("mqttport").toInt(), this);
        qDebug() << __PRETTY_FUNCTION__ << ": Using localhost";
    }
    connect(m_mqttClient, SIGNAL(connectionComplete()), this, SLOT(connectionComplete()));
    connect(m_mqttClient, SIGNAL(disconnectedEvent()), this, SLOT(disconnectedEvent()));
    connect(m_mqttClient, SIGNAL(messageReceivedOnTopic(QString, QString)), this, SLOT(messageReceivedOnTopic(QString, QString)));
    m_mqttClient->connectToHost();
    m_lightningTimer = new QTimer();
    connect(m_lightningTimer, SIGNAL(timeout()), this, SLOT(lightningTimeout()));
}

void PrimaryDisplay::setNYETimeout()
{
    QDateTime dt = QDateTime::currentDateTime();
    QTime t(23, 59, 0);
    QDate d(dt.date().year(), 12, 31);
    QDateTime nye_mseconds(d, t);
    int timeout = static_cast<int>(dt.msecsTo(nye_mseconds));     // Forcing the conversion to an int, that's what QTimer takes

    if (timeout == 0) {
        emit startNYE();
    }
    else if (timeout < 0) {
        QTimer::singleShot(10000000, this, SLOT(setNYETimeout()));
    }
    else {
        QTimer::singleShot(timeout, this, SLOT(showNYECountDown()));
    }
}

void PrimaryDisplay::showNYECountDown()
{
    emit startNYE();
}

void PrimaryDisplay::updateClock()
{
    QTime t = QTime::currentTime();
    QDate d = QDate::currentDate();


    m_primaryDate->setText(d.toString("dddd MMMM d, yyyy"));
    m_primaryClock->setText(t.toString("h:mm A"));
}

void PrimaryDisplay::enableBacklight(bool state)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    
    if (settings.contains("backlight")) {
        QString sysfs = settings.value("backlight").toString();
        QFile bl(sysfs);
        if (bl.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&bl);
            if (state == true)
                ts << "255";
            else
                ts << "0";
            bl.close();
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << ": Backlight not found at" << sysfs;
        }
    }
}

void PrimaryDisplay::showPrimaryScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
}

void PrimaryDisplay::updateNYEClock()
{
    QTime t = QTime::currentTime();
    QString countdown("<font style='font-size:200px; color:white; font-weight: bold;'>%1</font>");
    m_lbCountdown->setText(countdown.arg(60 - t.second()));
}

void PrimaryDisplay::showNYEScreen()
{
    QTime t = QTime::currentTime();

    m_stackedWidget->setCurrentIndex(WidgetIndex::NYE);
    
    if (t.hour() == 23) {
        QString countdown("<font style='font-size:200px; color:white; font-weight: bold;'>%1</font>");
        m_lbCountdown->setText(countdown.arg(60 - t.second()));
        QTimer::singleShot(1000, this, SLOT(updateNYEClock()));
    }
    else
        emit stopNYE();
}

void PrimaryDisplay::connectionComplete()
{
    m_mqttClient->subscribe("weather/#");
}

void PrimaryDisplay::disconnectedEvent()
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT connection lost";
    m_mqttClient->connectToHost();
}

void PrimaryDisplay::lightningTimeout()
{
}

void PrimaryDisplay::endMetadataScreen()
{
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);    
}

void PrimaryDisplay::showMetadataScreen()
{
    m_stackedWidget->setCurrentIndex(WidgetIndex::Sonos);
}

void PrimaryDisplay::endWeatherScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_endWeatherScreen->stop();
    emit hideWeatherScreen();
}

void PrimaryDisplay::showWeatherScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_endWeatherScreen->setSingleShot(true);
    m_endWeatherScreen->setInterval(1000 * 120);
    m_endWeatherScreen->start();
    m_stackedWidget->setCurrentIndex(WidgetIndex::Weather);
}

void PrimaryDisplay::showBlankScreen()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    int interval = ONE_HOUR * 4;
    
    if (settings.contains("blankinterval")) {
        interval = settings.value("blankinterval").toInt();
    }
    m_stackedWidget->setCurrentIndex(WidgetIndex::Blank);
    enableBacklight(false);
    qDebug() << __PRETTY_FUNCTION__ << ": sleeping for" << interval / 1000 << "seconds";
    m_endBlankScreen->setInterval(interval);
    m_endBlankScreen->setSingleShot(true);
    m_endBlankScreen->start();
}

void PrimaryDisplay::endBlankScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    enableBacklight(true);
    setupBlankScreenTimers();
}

void PrimaryDisplay::messageReceivedOnTopic(QString t, QString p)
{
    QJsonDocument doc = QJsonDocument::fromJson(p.toLocal8Bit());
    
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        m_weatherWidget->updateDisplay(t, parent);
        if (t == "weather/conditions") {
            QJsonObject values = parent["environment"].toObject();
            double t = values["farenheit"].toDouble();
            double h = values["humidity"].toDouble();
            
            QString temp = QString("%1%2").arg(t, 0, 'f', 1).arg(QChar(176));
            QString humidity = QString("%1%").arg(h, 0, 'f', 1);
            m_temperature->setText(temp);
            m_humidity->setText(humidity);
        }
        else if (t == "weather/rainfall") {
            if (parent.contains("today")) {
                m_rainLabel->setText(QString("%1 in").arg(parent["today"].toDouble(), 0, 'f', 2));
            }
        }
        else if (t == "weather/light") {
            if (parent.contains("uv")) {
                int uv = parent["uv"].toInt();
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
}    
