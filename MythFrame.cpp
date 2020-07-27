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

#include "MythFrame.h"

MythFrame::MythFrame(QFrame *parent) : QFrame(parent) {
	QPalette pal(QColor(0,0,0));
	setBackgroundRole(QPalette::Window);
	pal.setColor(QPalette::Window, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

    m_primaryLayoutWidget = new QWidget();
    m_primaryLayout = new QGridLayout(m_primaryLayoutWidget);
    m_primaryLayout->setMargin(0);
    m_primaryLayout->setSpacing(0);
	m_primaryClock = new QLabel();
    m_primaryClock->setAlignment(Qt::AlignCenter);
    m_primaryDate = new QLabel();
    m_primaryDate->setAlignment(Qt::AlignCenter);
    m_lightningLabel = new QLabel();
    m_temperature = new QLabel();
    m_humidity = new QLabel();
    m_temperature->setAlignment(Qt::AlignCenter);
    m_humidity->setAlignment(Qt::AlignCenter);

    m_primaryLayout->addWidget(m_primaryClock, 0, 0, 1, 4);
    m_primaryLayout->addWidget(m_temperature, 2, 0, 1, 2);
    m_primaryLayout->addWidget(m_humidity, 2, 2, 1, 2);
    m_primaryLayout->addWidget(m_lightningLabel, 3, 0, 1, 4);
    m_primaryLayout->addWidget(m_primaryDate, 4, 0, 1, 4);
    
    m_nyeLayoutWidget = new QWidget();
    m_nyeLayout = new QGridLayout(m_nyeLayoutWidget);
    m_lbCountdown = new QLabel();
    m_nyeLayout->addWidget(m_lbCountdown, 0, 0, 0, 4);
    
    m_sonosLayoutWidget = new QWidget();
    m_sonosLayout = new QGridLayout(m_sonosLayoutWidget);
    m_title = new QLabel();
    m_artist = new QLabel();
    m_album = new QLabel();
    m_station = new QLabel();
    m_albumArt = new QLabel();
    m_elapsedIndicator = new QProgressBar();
    
    m_sonosLayout->addWidget(m_title, 0, 0, 2, 4);
    m_sonosLayout->addWidget(m_albumArt, 2, 0, 3, 1);
    m_sonosLayout->addWidget(m_artist, 2, 1, 1, 3);
    m_sonosLayout->addWidget(m_album, 3, 1, 1, 3);
    m_sonosLayout->addWidget(m_station, 4, 1, 1, 3);
    m_sonosLayout->addWidget(m_elapsedIndicator, 5, 0, 1, 4);
    
    QFont c("Roboto-Regular", 25);
    QFont l("Roboto-Regular", 20);
    QFont p("Roboto-Regular", 100);
    QFont d("Roboto-Regular", 40);

	m_primaryClock->setFont(p);
	m_primaryDate->setFont(d);
	m_temperature->setFont(c);
	m_humidity->setFont(c);
    m_artist->setFont(c);
    m_album->setFont(c);
    m_station->setFont(c);
    m_title->setFont(c);
    m_lightningLabel->setFont(l);
/*
    m_parentLayout = new QVBoxLayout();
    m_parentLayout->addWidget(m_primaryLayoutWidget);
    m_parentLayout->addWidget(m_sonosLayoutWidget);
    m_parentLayout->addWidget(m_nyeLayoutWidget);
*/
    m_stackedLayout = new QStackedLayout(this);
    m_stackedLayout->addWidget(m_primaryLayoutWidget);
    m_stackedLayout->addWidget(m_sonosLayoutWidget);
    m_stackedLayout->addWidget(m_nyeLayoutWidget);
    m_parentLayout = new QVBoxLayout();
    setLayout(m_parentLayout);
        
	m_clockTimer = new QTimer(this);
	connect(m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
	m_clockTimer->setInterval(50);
	m_clockTimer->start();

    m_sonos = new SonosRequest();
    connect(m_sonos, &SonosRequest::result, this, &MythFrame::sonosRequestResult);
    connect(m_sonos, &SonosRequest::error, this, &MythFrame::sonosRequestError);
    setupSonos();
    
    m_sonosTimer = new QTimer(this);
    connect(m_sonosTimer, &QTimer::timeout, this, &MythFrame::sonosUpdate);
    m_sonosTimer->setInterval(500);
    m_sonosTimer->start();

    setupMqttSubscriber();
    m_trackNumber = 0;
        
	QState *primary = new QState();
	QState *metadata = new QState();
	QState *nye = new QState();

	nye->addTransition(this, SIGNAL(stopNYE()), primary);
	primary->addTransition(this, SIGNAL(startSonos()), metadata);
	primary->addTransition(this, SIGNAL(startNYE()), nye);
	metadata->addTransition(this, SIGNAL(startNYE()), nye);
    metadata->addTransition(this, SIGNAL(endSonos()), primary);

	connect(metadata, SIGNAL(exited()), this, SLOT(hideMetadataScreen()));
	connect(metadata, SIGNAL(entered()), this, SLOT(showMetadataScreen()));
	connect(primary, SIGNAL(exited()), this, SLOT(hidePrimaryScreen()));
	connect(primary, SIGNAL(entered()), this, SLOT(showPrimaryScreen()));
	connect(nye, SIGNAL(exited()), this, SLOT(hideNYEScreen()));
	connect(nye, SIGNAL(entered()), this, SLOT(showNYEScreen()));

	m_states.addState(primary);
	m_states.addState(metadata);
	m_states.addState(nye);
	m_states.setInitialState(primary);

    setNYETimeout();

    m_states.start();
}

MythFrame::~MythFrame() 
{
}

void MythFrame::setupSonos()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    QString hostname = settings.value("sonosserver").toString();
    QHostInfo lookup = QHostInfo::fromName(hostname);
    QList<QHostAddress> addresses = lookup.addresses();
    
    if (addresses.size() > 0) {
        QString url = QString("%1:%2").arg(addresses.at(0).toString()).arg(settings.value("sonosport").toInt());
        m_sonos->setURL(url, settings.value("sonosroom").toString());
        qDebug() << __PRETTY_FUNCTION__ << ": setting host address to" << addresses.at(0);
    }
    else {
        m_sonos->setURL("http://localhost:5005", settings.value("sonosroom").toString());
        qDebug() << __PRETTY_FUNCTION__ << ": Using localhost";
    }
}

void MythFrame::sonosUpdate()
{
    if (!m_sonos->inProgress())
        m_sonos->run();
}

void MythFrame::sonosRequestError(QNetworkReply::NetworkError error)
{
    qDebug() << __FUNCTION__ << error;
}

void MythFrame::calculateMinutes(int elapsed)
{
    QString display = QString("%1:%2").arg(elapsed/60, 2, 10, QChar('0')).arg(elapsed%60, 2, 10, QChar('0'));
    m_elapsedIndicator->setFormat(display);
    m_elapsedIndicator->setValue(elapsed);
}

void MythFrame::sonosRequestResult(QByteArray ba)
{
    QJsonParseError *error = new QJsonParseError();
    
    QJsonDocument doc = QJsonDocument::fromJson(ba, error);
    if (error->error != QJsonParseError::NoError) {
        qDebug() << error->errorString();
    }
    else {
        if (doc.isObject()) {
            QJsonObject parent = doc.object();
            QString playback = parent["playbackState"].toString();
            if (playback == "PLAYING") {
                QJsonObject current = parent["currentTrack"].toObject();
                if (parent["trackNo"] != m_trackNumber) { 
                    m_artist->setText(current["artist"].toString());
                    m_album->setText(current["album"].toString());
                    m_station->setText(current["stationName"].toString());
                    m_duration = current["duration"].toInt();
                    m_title->setText(current["title"].toString());
                    m_elapsedIndicator->setMaximum(m_duration);
                    m_elapsedIndicator->setMinimum(0);
                    m_trackNumber = parent["trackNo"].toInt();
                }
                calculateMinutes(parent["elapsedTime"].toInt());
                m_volume = parent["volume"].toInt();
                emit startSonos();
            }
            else {
                emit endSonos();
            }
        }
    }
}

void MythFrame::setupMqttSubscriber()
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

void MythFrame::setNYETimeout()
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

void MythFrame::showNYECountDown()
{
	emit startNYE();
}

void MythFrame::updateClock()
{
	QTime t = QTime::currentTime();
	QDate d = QDate::currentDate();

    if (t.hour() > 1 && t.hour() < 5) {
        m_primaryDate->clear();
        m_primaryClock->clear();
    }
    else {
        QString dateDisplay("<font style='color:%1; font-weight: bold;'>%2</font>");
        QString largeDisplay("<font style='color:%1; font-weight: bold;'>%2</font>");
        m_primaryDate->setText(dateDisplay.arg(m_clockColor).arg(d.toString("dddd MMMM d, yyyy")));
        m_primaryClock->setText(largeDisplay.arg(m_clockColor).arg(t.toString("h:mm A")));
    }
}

void MythFrame::showEvent(QShowEvent *e)
{
	Q_UNUSED(e)
}

void MythFrame::hidePrimaryScreen()
{
    m_primaryLayoutWidget->hide();
}

void MythFrame::showPrimaryScreen()
{
    m_stackedLayout->setCurrentIndex(0);
}

void MythFrame::hideMetadataScreen()
{
    m_sonosLayoutWidget->hide();
}

void MythFrame::showMetadataScreen()
{
    m_stackedLayout->setCurrentIndex(1);
}

void MythFrame::showNYEScreen()
{
	QTime t = QTime::currentTime();

    m_nyeLayoutWidget->show();
    
	if (t.hour() == 23) {
		QString countdown("<font style='font-size:200px; color:white; font-weight: bold;'>%1</font>");
		m_lbCountdown->setText(countdown.arg(60 - t.second()));
		QTimer::singleShot(1000, this, SLOT(showNYEScreen()));
	}
	else
		emit stopNYE();
}

void MythFrame::hideNYEScreen()
{
    m_nyeLayoutWidget->hide();
}

void MythFrame::connectionComplete()
{
    m_mqttClient->subscribe("weather/#");
}

void MythFrame::disconnectedEvent()
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT connection lost";
    m_mqttClient->connectToHost();
}

void MythFrame::lightningTimeout()
{
    m_lightningLabel->clear();
}

void MythFrame::messageReceivedOnTopic(QString t, QString p)
{
    QJsonDocument doc = QJsonDocument::fromJson(p.toLocal8Bit());
    QColor color;

    if (t == "weather/conditions") {
        if (doc.isObject()) {
            QJsonObject parent = doc.object();
            QJsonObject values = parent["environment"].toObject();
            double t = values["farenheit"].toDouble();
            double h = values["humidity"].toDouble();
            
            QString temp = QString("%1%2").arg(t, 0, 'f', 1).arg(QChar(176));
            QString humidity = QString("%1%").arg(h, 0, 'f', 1);
            m_temperature->setText(temp);
            m_humidity->setText(humidity);
        }
    }
    else if (t == "weather/event/lightning") {
        QJsonObject object = doc.object();
        QJsonObject lightning = object["lightning"].toObject();
            
        double d = lightning["miles"].toDouble();

        if (d > 15) {
            color = Qt::green;
        }
        else if (d > 5) {
            color = Qt::yellow;
        }
        else {
            color = Qt::red;
        }

        m_lightningLabel->setTextFormat(Qt::RichText);
        m_lightningLabel->setText(QString("Lightning Detected <font color=\"%1\">%2</font> miles away").arg(color.name()).arg(d, 0, 'f', 1));
        m_lightningTimer->stop();
        m_lightningTimer->setInterval(1000 * 30);
        m_lightningTimer->start();
    }
}    

