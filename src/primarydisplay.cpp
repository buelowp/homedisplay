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
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    int widthSettings = settings.value("width", 800).toInt();
    int heightSettings = settings.value("height", 480).toInt();

    QPalette pal(QColor(0,0,0));
    setBackgroundRole(QPalette::Window);
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    m_primaryLayoutWidget = new QWidget();
    
    m_nyeLayoutWidget = new QWidget();
    m_nyeLayout = new QHBoxLayout(m_nyeLayoutWidget);
    m_lbCountdown = new QLabel(m_primaryLayoutWidget);
    m_lbCountdown->setScaledContents(true);
    m_nyeLayout->addWidget(m_lbCountdown);
    
    QFont c("Roboto-Regular", 36);
    QFont l("Roboto-Regular", 28);
    QFont p("Roboto-Regular", 100);
    QFont d("Roboto-Regular", 32);


    m_weatherWidget = new WeatherDisplay();
    m_weatherWidget->setFixedSize(widthSettings, heightSettings);
    m_sonosWidget = new SonosDisplay();
    m_sonosWidget->setFixedSize(widthSettings, heightSettings);
    m_clockWidget = new ClockDisplay();
    m_clockWidget->setFixedSize(widthSettings,heightSettings);
    m_bigClock = new BigClock();
    m_bigClock->setFixedSize(widthSettings, heightSettings);
    m_blankLayoutWidget = new QWidget();

    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->addWidget(m_clockWidget);
    m_stackedWidget->addWidget(m_nyeLayoutWidget);
    m_stackedWidget->addWidget(m_blankLayoutWidget);
    m_stackedWidget->addWidget(m_weatherWidget);
    m_stackedWidget->addWidget(m_bigClock);
    m_stackedWidget->addWidget(m_sonosWidget);

    m_endWeatherScreen = new QTimer(this);
    connect(m_endWeatherScreen, &QTimer::timeout, this, &PrimaryDisplay::endWeatherScreen);

    m_startBlankScreen = new QTimer(this);
    m_endBlankScreen = new QTimer(this);
    m_endDimScreen = new QTimer(this);
    m_startBigClockScreen = new QTimer(this);
    m_endBigClockScreen = new QTimer(this);
  
    setupMqttSubscriber();
        
    QState *primary = new QState();
    QState *metadata = new QState();
    QState *nye = new QState();
    QState *blank = new QState();
    QState *weather = new QState();
    QState *bigclock = new QState();

    nye->addTransition(this, SIGNAL(stopNYE()), primary);
    primary->addTransition(m_sonosWidget, SIGNAL(startSonos()), metadata);
    primary->addTransition(this, SIGNAL(startNYE()), nye);
    primary->addTransition(m_startBlankScreen, &QTimer::timeout, blank);
    primary->addTransition(this, SIGNAL(startWeather()), weather);
    primary->addTransition(m_startBigClockScreen, &QTimer::timeout, bigclock);
    metadata->addTransition(this, SIGNAL(startNYE()), nye);
    metadata->addTransition(this, &PrimaryDisplay::startWeather, weather);
    metadata->addTransition(m_sonosWidget, SIGNAL(endSonos()), primary);
    weather->addTransition(this, SIGNAL(hideWeatherScreen()), primary);
    blank->addTransition(m_endBlankScreen, SIGNAL(timeout()), primary);
    bigclock->addTransition(m_endBigClockScreen, &QTimer::timeout, primary);

    connect(metadata, SIGNAL(entered()), this, SLOT(showMetadataScreen()));
    connect(primary, SIGNAL(entered()), this, SLOT(showPrimaryScreen()));
    connect(nye, SIGNAL(entered()), this, SLOT(showNYEScreen()));
    connect(weather, SIGNAL(entered()), this, SLOT(showWeatherScreen()));
    connect(bigclock, &QState::entered, this, &PrimaryDisplay::showBigClock);
    connect(blank, &QState::entered, this, &PrimaryDisplay::showBlankScreen);

    m_states.addState(primary);
    m_states.addState(metadata);
    m_states.addState(nye);
    m_states.addState(blank);
    m_states.addState(weather);
    m_states.addState(bigclock);
    m_states.setInitialState(primary);

    setNYETimeout();

    m_lux = new Lux();
    if (settings.value("usetsl2561").toBool()) {
        connect(m_lux, &Lux::lux, this, &PrimaryDisplay::lux);
        if (m_lux->isOpen()) {
            qDebug() << __PRETTY_FUNCTION__ << ": I can sense light";
            m_lux->go();
        }
    }

    int interval = getNightScreenTransitionTime();
    if (settings.value("blankscreen").toBool()) {
        qDebug() << __PRETTY_FUNCTION__ << ": Blank screen is enabled";
        connect(blank, SIGNAL(entered()), this, SLOT(showBlankScreen()));
        connect(blank, SIGNAL(exited()), this, SLOT(endBlankScreen()));
        m_startBlankScreen->setInterval(interval);
        m_startBlankScreen->setSingleShot(true);
        m_startBlankScreen->start();
    }

    if (settings.value("bigclock").toBool()) {
        qDebug() << __PRETTY_FUNCTION__ << ": Big clock is enabled";
        connect(bigclock, &QState::entered, this, &PrimaryDisplay::showBigClock);
        connect(bigclock, &QState::exited, this, &PrimaryDisplay::endBigClock);
        m_startBigClockScreen->setInterval(interval);
        m_startBigClockScreen->setSingleShot(true);
        m_startBigClockScreen->start();
    }

    enableBacklight(true);
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    setCentralWidget(m_stackedWidget);
    m_states.start();
    if (settings.value("usesonos").toBool() == true)
        m_sonosWidget->go();
}

PrimaryDisplay::~PrimaryDisplay() 
{
}

void PrimaryDisplay::showEvent(QShowEvent* event)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << width() << "x" << height();
    m_weatherWidget->setFixedSize(width(), height());
    m_sonosWidget->setFixedSize(width(), height());
    m_clockWidget->setFixedSize(width(), height());
}

int PrimaryDisplay::getNightScreenTransitionTime()
{
    QDateTime now = QDateTime::currentDateTime();
    int interval = 1000;
    
    if (now.time().hour() >= 1 && now.time().hour() < 3) {
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

    qDebug() << __PRETTY_FUNCTION__ << ": Night screen in" << interval / 1000 << "seconds";
    return interval;
}

void PrimaryDisplay::setupMqttSubscriber()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
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
}

void PrimaryDisplay::lux(long l)
{
    QDateTime now = QDateTime::currentDateTime();

    long bright = myMap(l, 0, 255, 10, 255);
    if (bright == 0)
        bright = 10;

    if (now.time().hour() >= 7 && now.time().hour() <= 21) {
        bright = 255;
    }

    if (bright < 10) {
        bright = 10;
    }
    if (bright != m_lastBrightValue) {
//        setBacklight(true, bright);
        m_lastBrightValue = bright;
    }
}

void PrimaryDisplay::setBacklight(bool state, uint8_t brightness)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");

    if (settings.contains("backlight")) {
        QString sysfs = settings.value("backlight").toString();
        QFile bl(sysfs);
        if (bl.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&bl);
            if (state == true)
                ts << brightness;
            else
                ts << "0";

            bl.close();
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << ": Backlight:" << sysfs << "" << bl.errorString();
        }
    }
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

void PrimaryDisplay::enableBacklight(bool state, uint8_t brightness)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    
    if (settings.contains("backlight")) {
        QString sysfs = settings.value("backlight").toString();
        QFile bl(sysfs);
        if (bl.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&bl);
            if (state == true)
                ts << brightness;
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
    m_stackedWidget->move(0,0);
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
    m_mqttClient->subscribe("garden/#");
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
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);    
}

void PrimaryDisplay::showMetadataScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Metadata);
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
    m_endWeatherScreen->setInterval(1000 * 60);
    m_endWeatherScreen->start();
    m_stackedWidget->setCurrentIndex(WidgetIndex::Weather);
}

void PrimaryDisplay::showBlankScreen()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    int interval = ONE_HOUR * 2;
    qDebug() << __PRETTY_FUNCTION__ << ": sleeping for" << interval / 1000 << "seconds";
    
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

void PrimaryDisplay::endDimScreen()
{
    enableBacklight(true);
    qDebug() << __PRETTY_FUNCTION__;
}

void PrimaryDisplay::showDimScreen()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    int interval = ONE_HOUR * 2;
    int dim = 100;
    qDebug() << __PRETTY_FUNCTION__ << ": sleeping for" << interval / 1000 << "seconds";

    if (settings.contains("diminterval")) {
        interval = settings.value("diminterval").toInt();
    }
    if (settings.contains("dimvalue")) {
        dim = settings.value("dimvalue").toInt();
    }
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    enableBacklight(true, dim);
    m_endDimScreen->setInterval(interval);
    m_endDimScreen->setSingleShot(true);
    m_endDimScreen->start();
    m_stackedWidget->setCurrentIndex(WidgetIndex::Bigclock);
}

void PrimaryDisplay::endBlankScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_startBlankScreen->setInterval(getNightScreenTransitionTime());
    m_startBlankScreen->setSingleShot(true);
    m_startBlankScreen->start();
}

void PrimaryDisplay::showBigClock()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Bigclock);
    m_endBigClockScreen->setInterval(FIVE_HOURS);
    m_endBigClockScreen->setSingleShot(true);
    m_endBigClockScreen->start();
}

void PrimaryDisplay::endBigClock()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    m_startBigClockScreen->setInterval(getNightScreenTransitionTime());
    m_startBigClockScreen->setSingleShot(true);
    m_startBigClockScreen->start();
}

void PrimaryDisplay::messageReceivedOnTopic(QString t, QString p)
{
    QJsonDocument doc = QJsonDocument::fromJson(p.toLocal8Bit());
    
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        m_weatherWidget->updateDisplay(t, parent);
        m_clockWidget->updateDisplay(t, parent);
    }
}    
