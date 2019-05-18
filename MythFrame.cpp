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

	m_primaryClock = new QLabel(this);
	m_server = new QTcpServer(this);
	m_metaChannel = new QLabel(this);
	m_metaTitle = new QLabel(this);
	m_metaShow = new QLabel(this);
	m_metaAudioImage = new QLabel(this);
	m_metaStereoImage = new QLabel(this);
	m_metaFlags = new QLabel(this);
	m_metaClock = new QLabel(this);
	m_primaryDate = new QLabel(this);
	m_metaTimeElapsed = new QLabel(this);
	m_metaTime = new QLabel(this);
	m_metaProgressBar = new QProgressBar(this);
	m_lbCountdown = new QLabel(this);
    m_lightningLabel = new QLabel(this);

	m_clockTimer = new QTimer(this);
	connect(m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
	m_clockTimer->setInterval(50);
	m_clockTimer->start();

	// Set the Myth connection indicator background black.
	m_metaChannel->setAutoFillBackground(true);
	m_metaChannel->setPalette(pal);
	m_metaTitle->setAutoFillBackground(true);
	m_metaTitle->setPalette(pal);
	m_metaShow->setAutoFillBackground(true);
	m_metaShow->setPalette(pal);
	m_metaAudioImage->setAutoFillBackground(true);
	m_metaAudioImage->setPalette(pal);
	m_metaStereoImage->setAutoFillBackground(true);
	m_metaStereoImage->setPalette(pal);
	m_metaFlags->setAutoFillBackground(true);
	m_metaFlags->setPalette(pal);
	m_metaClock->setAutoFillBackground(true);
	m_metaClock->setPalette(pal);
    
    m_kodi = new KodiLcdServer(this);
    setupKodi();

	m_disableProgressIndicator = false;
	m_clockColor = "#FFFFFF";
    
    connect(m_server, SIGNAL(newConnection()), this, SLOT(connCreated()));
    setupMqttSubscriber();
}

MythFrame::~MythFrame() {
    if (m_mythLcd) {
        m_mythLcd->close();
        m_mythLcd->deleteLater();
    }
	m_server->close();
	m_server->deleteLater();;
}

void MythFrame::setupKodi()
{
    connect(m_kodi, SIGNAL(clientConnected()), this, SLOT(kodiConnected()));
	connect(m_kodi, SIGNAL(channelNumber(QByteArray)), this, SLOT(channelUpdate(QByteArray)));
	connect(m_kodi, SIGNAL(showTitle(QByteArray)), this, SLOT(showTitle(QByteArray)));
	connect(m_kodi, SIGNAL(showSubTitle(QByteArray)), this, SLOT(showSubTitle(QByteArray)));
	connect(m_kodi, SIGNAL(progressTimeLeft(QByteArray)), this, SLOT(elapsedTime(QByteArray)));
	connect(m_kodi, SIGNAL(progressTotalTime(QByteArray)), this, SLOT(totalTime(QByteArray)));
	connect(m_kodi, SIGNAL(progressPercentComplete(double)), this, SLOT(percentComplete(double)));
	connect(m_kodi, SIGNAL(videoFormat(QString)), this, SLOT(videoFormat(QString)));
	connect(m_kodi, SIGNAL(audioFormat(QString)), this, SLOT(audioFormat(QString)));
	connect(m_kodi, SIGNAL(stereoFormat(QString)), this, SLOT(stereoFormat(QString)));
	connect(m_kodi, SIGNAL(playbackFlags(QString)), this, SLOT(playbackFlags(QString)));
	connect(m_kodi, SIGNAL(metaDataStarted()), this, SLOT(metaDataStarted()));
	connect(m_kodi, SIGNAL(metaDataEnded()), this, SLOT(metaDataEnded()));
}

void MythFrame::kodiConnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Connected to Kodi media server";
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
    int timeout = (int)dt.msecsTo(nye_mseconds);     // Forcing the conversion to an int, that's what QTimer takes

    if (timeout == 0) {
        emit startNYE();
    }
    else if (timeout < 0) {
//        qDebug() << __PRETTY_FUNCTION__ << ": Setting timeout to 1000000 and will retry to set the acutal timeout then";
        QTimer::singleShot(10000000, this, SLOT(setNYETimeout()));
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ": Setting timeout to" << timeout;
        QTimer::singleShot(timeout, this, SLOT(showNYECountDown()));
    }
}

bool MythFrame::init()
{
	QState *primary = new QState();
	QState *metadata = new QState();
	QState *nye = new QState();
    QState *lightning = new QState();

	metadata->addTransition(this, SIGNAL(lcdDisconnect()), primary);
	metadata->addTransition(this, SIGNAL(videoPlaybackEnded()), primary);
	nye->addTransition(this, SIGNAL(nyeEventDone()), primary);
	primary->addTransition(this, SIGNAL(videoPlaybackStarted()), metadata);
	primary->addTransition(this, SIGNAL(startNYE()), nye);
    primary->addTransition(this, SIGNAL(startLightning()), lightning);
	metadata->addTransition(this, SIGNAL(startNYE()), nye);
    lightning->addTransition(this, SIGNAL(endLightning()), primary);

	connect(metadata, SIGNAL(entered()), this, SLOT(metaDataStarted()));
	connect(metadata, SIGNAL(exited()), this, SLOT(hideMetadataScreen()));
	connect(primary, SIGNAL(exited()), this, SLOT(hidePrimaryScreen()));
	connect(nye, SIGNAL(exited()), this, SLOT(hideNYEScreen()));
	connect(metadata, SIGNAL(entered()), this, SLOT(showMetadataScreen()));
	connect(primary, SIGNAL(entered()), this, SLOT(showPrimaryScreen()));
	connect(nye, SIGNAL(entered()), this, SLOT(showNYEScreen()));
    connect(lightning, SIGNAL(entered()), this, SLOT(showLightningScreen()));
    connect(lightning, SIGNAL(exited()), this, SLOT(hideLightningScreen()));

	m_states.addState(primary);
	m_states.addState(metadata);
	m_states.addState(nye);
    m_states.addState(lightning);
	m_states.setInitialState(primary);

    hidePrimaryScreen();
    hideNYEScreen();
    hideMetadataScreen();
    hideLightningScreen();
    
    setNYETimeout();

    m_states.start();
    m_kodi->exec();
    
	return m_server->listen(QHostAddress::Any, 6545);
}

void MythFrame::showNYECountDown()
{
	emit startNYE();
}

void MythFrame::showNYEScreen()
{
	QTime t = QTime::currentTime();

	m_lbCountdown->show();

	if (t.hour() == 23) {
		QString countdown("<font style='font-size:200px; color:white; font-weight: bold;'>%1</font>");
		m_lbCountdown->setText(countdown.arg(60 - t.second()));
        m_lbCountdown->setAlignment(Qt::AlignCenter);
		m_lbCountdown->show();
		QTimer::singleShot(1000, this, SLOT(showNYEScreen()));
	}
	else
		emit stopNYE();
}

void MythFrame::updateClock()
{
	QTime t = QTime::currentTime();
	QDate d = QDate::currentDate();

    if (t.hour() > 1 && t.hour() < 5) {
        m_metaClock->clear();
        m_primaryDate->clear();
        m_primaryClock->clear();
    }
    else {
        QString smallDisplay("<font style='font-size:50px; color:white; font-weight: bold;'>%1</font>");
        QString dateDisplay("<font style='font-size:45px; color:%1; font-weight: bold;'>%2</font>");
        QString largeDisplay("<font style='font-size:140px; color:%1; font-weight: bold;'>%2</font>");
        m_metaClock->setText(smallDisplay.arg(t.toString("h:mm a")));
        m_primaryDate->setText(dateDisplay.arg(m_clockColor).arg(d.toString("dddd MMMM d, yyyy")));
        m_primaryClock->setText(largeDisplay.arg(m_clockColor).arg(t.toString("h:mm A")));
    }
}

void MythFrame::connCreated()
{
	m_mythLcd = new MythLcdServer(m_server->nextPendingConnection());

	connect(m_mythLcd, SIGNAL(sockClosed()), this, SLOT(connClosed()));
	connect(m_mythLcd, SIGNAL(channelNumber(QByteArray)), this, SLOT(channelUpdate(QByteArray)));
	connect(m_mythLcd, SIGNAL(showTitle(QByteArray)), this, SLOT(showTitle(QByteArray)));
	connect(m_mythLcd, SIGNAL(showSubTitle(QByteArray)), this, SLOT(showSubTitle(QByteArray)));
	connect(m_mythLcd, SIGNAL(progressTimeLeft(QByteArray)), this, SLOT(elapsedTime(QByteArray)));
	connect(m_mythLcd, SIGNAL(progressTotalTime(QByteArray)), this, SLOT(totalTime(QByteArray)));
	connect(m_mythLcd, SIGNAL(progressPercentComplete(int)), this, SLOT(percentComplete(int)));
	connect(m_mythLcd, SIGNAL(videoFormat(QString)), this, SLOT(videoFormat(QString)));
	connect(m_mythLcd, SIGNAL(audioFormat(QString)), this, SLOT(audioFormat(QString)));
	connect(m_mythLcd, SIGNAL(stereoFormat(QString)), this, SLOT(stereoFormat(QString)));
	connect(m_mythLcd, SIGNAL(playbackFlags(QString)), this, SLOT(playbackFlags(QString)));
	connect(m_mythLcd, SIGNAL(metaDataStarted()), this, SLOT(metaDataStarted()));
	connect(m_mythLcd, SIGNAL(metaDataEnded()), this, SLOT(metaDataEnded()));
    connect(m_mythLcd, SIGNAL(mythConnected()), this, SLOT(mythConnected()));
    connect(m_mythLcd, SIGNAL(liveTV()), this, SLOT(disableProgressIndicator()));
    connect(m_mythLcd, SIGNAL(recordedEvent()), this, SLOT(enableProgressIndicator()));
}

void MythFrame::mythConnected()
{
    m_clockColor = "#FFFFFF";
    emit toConnectedState();
}

void MythFrame::videoFormat(QString)
{
}

void MythFrame::audioFormat(QString v)
{
//	qDebug() << __PRETTY_FUNCTION__ << ":" << v;
	if (v == "dts") {
        m_metaAudioImage->setPixmap(QPixmap("/usr/share/mythclock/dts.png"));
	}
    else if (v == "ac3") {
        m_metaAudioImage->setPixmap(QPixmap("/usr/share/mythclock/ac3.png"));
    }
	else {
		m_metaAudioImage->setText(v);
	}
}

void MythFrame::stereoFormat(QString f)
{
	m_metaStereoImage->setFont(QFont("Roboto-Regular", 12));
	m_metaStereoImage->setAlignment(Qt::AlignCenter);

//	qDebug() << __PRETTY_FUNCTION__ << ":" << f;
	if (f.compare("stereo", Qt::CaseInsensitive) == 0) {
        m_metaStereoImage->setPixmap(QPixmap("/usr/share/mythclock/stereo.png"));
	}
	if (f == "5.1") {
		m_metaStereoImage->setPixmap(QPixmap("/usr/share/mythclock/5_1.jpg"));
	}
	if (f == "7.1") {
		m_metaStereoImage->setPixmap(QPixmap("/usr/share/mythclock/7_1.jpg"));
	}
}

void MythFrame::playbackFlags(QString flags)
{
	QPixmap hd("/usr/share/mythclock/HD.png");

//	qDebug() << __PRETTY_FUNCTION__ << ":" << flags;
	if (flags == "Hi-Def") {
		if (hd.isNull())
			qDebug() << "Error opening hd pixmap";

		m_metaFlags->setPixmap(hd);
	}
	else {
		QFont f("Roboto-Regular", 20);
		m_metaFlags->setFont(f);
		m_metaFlags->setText(flags);
	}
	m_metaFlags->setAlignment(Qt::AlignCenter);
}

void MythFrame::metaDataStarted()
{
    if (m_disableProgressIndicator) {
        m_metaProgressBar->show();
        m_metaTime->show();
        m_metaTimeElapsed->show();
    }
    else {
        m_metaProgressBar->hide();
        m_metaTime->hide();
        m_metaTimeElapsed->hide();
    }
	emit videoPlaybackStarted();
}

void MythFrame::metaDataEnded()
{
    m_metaShow->clear();
    m_metaTime->clear();
    m_metaFlags->clear();
    m_metaTitle->clear();
    m_metaChannel->clear();
    m_metaProgressBar->reset();
    m_metaTimeElapsed->clear();
    m_disableProgressIndicator = false;
	emit videoPlaybackEnded();
}

void MythFrame::connClosed()
{
//    qDebug() << __PRETTY_FUNCTION__;
	m_mythLcd->deleteLater();
	emit lcdDisconnect();
}

void MythFrame::channelUpdate(QByteArray s)
{
    QString style = QString("<font style='font-size:30px; color:white;'>%1</font>");
//    qDebug() << __PRETTY_FUNCTION__ << ":" << s;
	m_metaChannel->setText(style.arg(s.data()));
}

void MythFrame::showTitle(QByteArray s)
{
    QString style = QString("<font style='font-size:50px; color:white; font-weight: bold;'>%1</font>");

    qDebug() << __PRETTY_FUNCTION__ << ":" << s;
    m_metaShow->setText(style.arg(s.data()));
}

void MythFrame::showSubTitle(QByteArray s)
{
    QString style = QString("<font style='font-size:30px; color:white;'>%1</font>");
//    qDebug() << __PRETTY_FUNCTION__ << ":" << s;
    m_metaTitle->setText(style.arg(s.data()));
}

void MythFrame::elapsedTime(QByteArray ba)
{
    QString format = QString("<font style='font-size:30px; color:white;'>%1:%2:%3</font>");
    QList<QByteArray> segments = ba.split(':');
    
    if (m_disableProgressIndicator)
        return;
    
//	qDebug() << __PRETTY_FUNCTION__ << ":" << ba;
    if (segments.size() == 2) {
        QString m = segments.at(0).data();
        QString s = segments.at(1).data();
        m_metaTimeElapsed->setText(format.arg("0").arg(m, 2, '0').arg(s, 2, '0'));
    }
    if (segments.size() == 3) {
        QString h = segments.at(0).data();
        QString m = segments.at(1).data();
        QString s = segments.at(2).data();
        m_metaTimeElapsed->setText(format.arg(h, 1, '0').arg(m, 2, '0').arg(s, 2, '0'));
    }
}

void MythFrame::totalTime(QByteArray ba)
{
    QString format = QString("<font style='font-size:30px; color:white;'>%1:%2:%3</font>");
    QList<QByteArray> segments = ba.split(':');
    
    if (m_disableProgressIndicator)
        return;
    
//	qDebug() << __PRETTY_FUNCTION__ << ":" << ba;
    if (segments.size() == 2) {
        QString m = segments.at(0).data();
        QString s = segments.at(1).data();
        m_metaTime->setText(format.arg("0").arg(m, 2, '0').arg(s, 2, '0'));
    }
    if (segments.size() == 3) {
        QString h = segments.at(0).data();
        QString m = segments.at(1).data();
        QString s = segments.at(2).data();
        m_metaTime->setText(format.arg(h, 1, '0').arg(m, 2, '0').arg(s, 2, '0'));
    }
}

void MythFrame::percentComplete(int p)
{
    if (!m_disableProgressIndicator) {
        m_metaProgressBar->setValue(p);
    }
}

void MythFrame::percentComplete(double p)
{
    if (!m_disableProgressIndicator) {
        m_metaProgressBar->setValue(static_cast<int>(p));
    }
}

void MythFrame::disableProgressIndicator()
{
//    qDebug() << __PRETTY_FUNCTION__;
    m_disableProgressIndicator = true;
}

void MythFrame::enableProgressIndicator()
{
//    qDebug() << __PRETTY_FUNCTION__;
    m_disableProgressIndicator = false;
}

void MythFrame::showEvent(QShowEvent *e)
{
	Q_UNUSED(e)
    QFont c("Roboto-Regular", 15);

	m_primaryClock->setGeometry(0, 0, 800, 320);
	m_primaryDate->setGeometry(0, 320, 800, 160);

    m_metaShow->setGeometry(0, 0, 800, 200);
	m_metaTitle->setGeometry(0, 200, 600, 80);
	m_metaChannel->setGeometry(0, 280, 600, 80);
	m_metaClock->setGeometry(0, 360, 600, 80);
	m_metaProgressBar->setGeometry(0, 440, 800, 40);

	m_metaAudioImage->setGeometry(600, 200, 100, 60);
	m_metaStereoImage->setGeometry(700, 200, 100, 60);
	m_metaFlags->setGeometry(650, 260, 100, 60);
	m_metaTimeElapsed->setGeometry(600, 320, 200, 60);
	m_metaTime->setGeometry(600, 380, 200, 60);

	m_lbCountdown->setGeometry(0, 0, 480, 800);

	m_metaTime->setAlignment(Qt::AlignCenter);
	m_metaTimeElapsed->setAlignment(Qt::AlignCenter);
	m_lbCountdown->setAlignment(Qt::AlignCenter);
	m_metaClock->setAlignment(Qt::AlignCenter);
	m_primaryDate->setAlignment(Qt::AlignCenter);
    m_primaryClock->setAlignment(Qt::AlignCenter);

	m_primaryClock->setFont(c);
	m_primaryDate->setFont(c);
	m_metaClock->setFont(c);
	m_metaTitle->setFont(c);
	m_metaShow->setFont(c);
	m_metaShow->setIndent(10);
	m_metaTitle->setIndent(10);
	m_metaTitle->setFont(c);
	m_metaChannel->setIndent(10);
	m_metaChannel->setFont(c);

	m_metaProgressBar->setStyleSheet(".QProgressBar{border: 2px solid black; background: black; padding: 2px;} QProgressBar::chunk{border-radius: 3px; background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #fff, stop: .25 #fee, stop: .5 #fbb, stop: .75 #f66, stop: 1 #f00);}");
	m_metaTime->setStyleSheet(".QLabel{font-size:50px; background-color: black; color: white;}");
	m_metaTimeElapsed->setStyleSheet(".QLabel{font-size:50px; background-color: black; color: white;}");

	m_metaProgressBar->setRange(0, 100);
	m_metaProgressBar->setTextVisible(false);
    
    QFont l("Roboto-Regular", 45);
    m_lightningLabel->setFont(l);
    m_lightningLabel->setGeometry(0, 140, 800, 200);
}

void MythFrame::hidePrimaryScreen()
{
//    qDebug() << __PRETTY_FUNCTION__;
	m_primaryClock->hide();
	m_primaryDate->hide();
}

void MythFrame::hideMetadataScreen()
{
//    qDebug() << __PRETTY_FUNCTION__;
	m_metaClock->hide();
	m_metaTitle->hide();
	m_metaShow->hide();
	m_metaChannel->hide();
	m_metaAudioImage->hide();
	m_metaStereoImage->hide();
	m_metaFlags->hide();
	m_metaTimeElapsed->hide();
	m_metaTime->hide();
	m_metaProgressBar->hide();
}

void MythFrame::showMetadataScreen()
{
//    qDebug() << __PRETTY_FUNCTION__;
	m_metaClock->show();
	m_metaTitle->show();
	m_metaShow->show();
	m_metaChannel->show();
	m_metaAudioImage->show();
	m_metaStereoImage->show();
	m_metaFlags->show();
	m_metaTimeElapsed->show();
	m_metaTime->show();
	m_metaProgressBar->show();
}

void MythFrame::showPrimaryScreen()
{
//    qDebug() << __PRETTY_FUNCTION__;
	m_primaryClock->show();
	m_primaryDate->show();
}

void MythFrame::hideNYEScreen()
{
//    qDebug() << __PRETTY_FUNCTION__;
	m_lbCountdown->hide();
}

void MythFrame::hideLightningScreen()
{
    m_lightningLabel->hide();
    m_lightningTimer->stop();
}

void MythFrame::disconnectClock()
{
    m_clockColor = "#FFC266";
    showPrimaryScreen();
}

void MythFrame::showLightningScreen()
{
    m_lightningLabel->show();
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
    emit endLightning();
}

void MythFrame::messageReceivedOnTopic(QString t, QString p)
{
    QJsonDocument doc = QJsonDocument::fromJson(p.toLocal8Bit());
    QColor color;

    if (!doc.isNull() && !doc.isEmpty()) {
        QJsonObject object = doc.object();
        if (object.contains("distance")) {
            double d = object["distance"].toString().toDouble();
            double distance = d * .621;
   
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
            m_lightningLabel->setAlignment(Qt::AlignCenter);
            m_lightningLabel->setText(QString("Lightning Detected<br><font color=\"%1\">%2</font> miles away").arg(color.name()).arg(distance, 0, 'f', 1));
            m_lightningTimer->stop();
            m_lightningTimer->setInterval(1000 * 30);
            m_lightningTimer->start();
            emit startLightning();
        }
    }
}    

