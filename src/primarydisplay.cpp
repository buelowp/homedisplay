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

PrimaryDisplay::PrimaryDisplay(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");

    QPalette pal(QColor(0,0,0));
    setBackgroundRole(QPalette::Window);
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    if (primaryScreen) {
        // Get the screen's full geometry (resolution in pixels)
        QRect screenGeometry = primaryScreen->geometry();
        m_width = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        qDebug() << __PRETTY_FUNCTION__ << "Screen Resolution:" << m_width << "x" << screenHeight;
    }

    setFixedSize(primaryScreen->geometry().width(), primaryScreen->geometry().height());
    m_maxBrightness = 255;
    
    QFont c("Roboto-Regular", 36);
    QFont l("Roboto-Regular", 28);
    QFont p("Roboto-Regular", 100);
    QFont d("Roboto-Regular", 32);

    m_weatherWidget = new WeatherDisplay();
    m_sonosWidget = new SonosDisplay();
    m_sonosWidget->setFixedSize(primaryScreen->geometry().width(), primaryScreen->geometry().height());
    m_clockWidget = new ClockDisplay();
    m_bigClock = new BigClock();
    m_nyeWidget = new NYEWidget();
    m_blankLayoutWidget = new QWidget();

    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->addWidget(m_clockWidget);
    m_stackedWidget->addWidget(m_nyeWidget);
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
    m_sonos = new Noson();
    if (settings.value("usesonos").toBool() == true) {
        connect(m_sonos, &Noson::title, m_sonosWidget, &SonosDisplay::updateTitle);
        connect(m_sonos, &Noson::artist, m_sonosWidget, &SonosDisplay::updateArtist);
        connect(m_sonos, &Noson::album, m_sonosWidget, &SonosDisplay::updateAlbum);
        connect(m_sonos, &Noson::duration, m_sonosWidget, &SonosDisplay::updateDuration);
        connect(m_sonos, &Noson::position, m_sonosWidget, &SonosDisplay::updatePosition);
        connect(m_sonos, &Noson::art, m_sonosWidget, &SonosDisplay::updateAlbumArt);
        m_sonos->go();
    }

    QState *primary = new QState();
    QState *sonos = new QState();
    QState *nye = new QState();
    QState *blank = new QState();
    QState *weather = new QState();
    QState *bigclock = new QState();

    nye->addTransition(m_nyeWidget, &NYEWidget::finished, primary);
    primary->addTransition(m_sonos, &Noson::startDisplay, sonos);
    primary->addTransition(this, SIGNAL(startNYE()), nye);
    primary->addTransition(m_startBlankScreen, &QTimer::timeout, blank);
    primary->addTransition(this, SIGNAL(startWeather()), weather);
    primary->addTransition(m_startBigClockScreen, &QTimer::timeout, bigclock);
    sonos->addTransition(this, SIGNAL(startNYE()), nye);
    sonos->addTransition(this, &PrimaryDisplay::startWeather, weather);
    sonos->addTransition(m_sonos, &Noson::endDisplay, primary);
    weather->addTransition(this, SIGNAL(hideWeatherScreen()), primary);
    blank->addTransition(m_endBlankScreen, SIGNAL(timeout()), primary);
    bigclock->addTransition(m_endBigClockScreen, &QTimer::timeout, primary);

    connect(sonos, SIGNAL(entered()), this, SLOT(showSonosScreen()));
    connect(primary, SIGNAL(entered()), this, SLOT(showPrimaryScreen()));
    connect(nye, SIGNAL(entered()), this, SLOT(showNYEScreen()));
    connect(weather, SIGNAL(entered()), this, SLOT(showWeatherScreen()));
    connect(bigclock, &QState::entered, this, &PrimaryDisplay::showBigClock);
    connect(blank, &QState::entered, this, &PrimaryDisplay::showBlankScreen);

    m_states.addState(primary);
    m_states.addState(sonos);
    m_states.addState(nye);
    m_states.addState(blank);
    m_states.addState(weather);
    m_states.addState(bigclock);
    m_states.setInitialState(primary);

    setNYETimeout();

    if (settings.value("usetsl2561").toBool()) {
        m_maxBrightness = settings.value("brightness", 255).toInt();
        qDebug() << __PRETTY_FUNCTION__ << ": Display has a max brightness value of" << m_maxBrightness;
        int bus = settings.value("tsl2561bus", 2).toInt();
        m_lux = new Lux(bus);
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

    m_environment = new Environment();
    connect(m_environment, &Environment::conditions, m_clockWidget, &ClockDisplay::updateLocalConditions);
    connect(m_environment, &Environment::conditions, this, &PrimaryDisplay::updateLocalConditions);
    m_environment->go();

    enableBacklight(true);
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    setCentralWidget(m_stackedWidget);
    m_states.start();
}

PrimaryDisplay::~PrimaryDisplay() 
{
}

bool PrimaryDisplay::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonRelease:
        emit startWeather();
        event->accept();
        return true; // Event handled
    }
    return QMainWindow::event(event);
}

void PrimaryDisplay::updateLocalConditions(double temp, double humidity)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    QMqttTopicName topic(QString("house/%1/environment").arg(settings.value("localconditionstopic").toString()));

    QJsonObject object;
    object["temperature"] = temp;
    object["humidity"] = humidity;
    QJsonDocument doc(object);
    m_mqttClient->publish(topic, doc.toJson());
}

void PrimaryDisplay::showEvent(QShowEvent* event)
{
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
    m_mqttClient = new QMqttClient;
    m_hostName = QString("%1-%2").arg(QHostInfo::localHostName()).arg(QRandomGenerator::global()->generate());
    m_mqttClient->setClientId(m_hostName);
    m_mqttClient->setAutoKeepAlive(true);
    
    connect(m_mqttClient, &QMqttClient::connected, this, &PrimaryDisplay::connected);
    connect(m_mqttClient, &QMqttClient::disconnected, this, &PrimaryDisplay::disconnected);
    connect(m_mqttClient, &QMqttClient::errorChanged, this, &PrimaryDisplay::errorChanged);
    connect(m_mqttClient, &QMqttClient::messageReceived, this, &PrimaryDisplay::messageReceived);
    m_mqttClient->setHostname(settings.value("mqttserver", "172.24.1.2").toString());
    m_mqttClient->setPort(settings.value("mqttport", 1883).toInt());
    m_mqttClient->connectToHost();
}

void PrimaryDisplay::lux(long l)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    QDateTime now = QDateTime::currentDateTime();
    int bright = 0;

    if (now.time().hour() >= 7 && now.time().hour() <= 21) {
        bright = m_maxBrightness;
    }
    else {
        bright = myMap(l, 0, 255, 1, m_maxBrightness);
        if (bright == 0)
            bright = 1;
    }

    if (bright != m_lastBrightValue) {
        setBacklight(true, bright);
        m_lastBrightValue = bright;
    }
}

void PrimaryDisplay::setBacklight(bool state, uint8_t brightness)
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
                ts << m_maxBrightness;

            bl.close();
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << ": Backlight:" << sysfs << "" << bl.errorString();
        }
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ": Settings does not contain backlight";
        qDebug() << __PRETTY_FUNCTION__ << ":" << settings.allKeys();
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
}

void PrimaryDisplay::showNYEScreen()
{
    m_stackedWidget->setCurrentIndex(WidgetIndex::NYE);
    m_nyeWidget->countdown();
}

void PrimaryDisplay::connected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT client connected";
    m_mqttClient->subscribe(QMqttTopicFilter("weather/#"));
    m_mqttClient->subscribe(QMqttTopicFilter("garden/#"));
    m_mqttClient->publish(QMqttTopicName("request/rainfall"), QByteArray("{\"request\": \"rainfall\"}"));
}

void PrimaryDisplay::disconnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT connection lost";
}

void PrimaryDisplay::errorChanged(QMqttClient::ClientError error)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << m_mqttClient->hostname() << ":" << m_mqttClient->port();
    qDebug() << __PRETTY_FUNCTION__ << ":" << error;
}

void PrimaryDisplay::endSonosScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    m_sonosWidget->stopTime();
}

void PrimaryDisplay::showSonosScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_stackedWidget->setCurrentIndex(WidgetIndex::Sonos);
    m_sonosWidget->startTime();
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
    enableBacklight(true);
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

void PrimaryDisplay::messageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message, &error);
    QString tname = topic.name();
    
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        m_weatherWidget->updateDisplay(tname, parent);
        m_clockWidget->updateDisplay(tname, parent);
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ":" << error.errorString();
    }
}    
