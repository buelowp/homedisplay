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

	m_clockColor = "#FFC266";

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

	bDisableProgress = false;
	m_clockColor = "#FFC266";
    
    connect(m_server, SIGNAL(newConnection()), this, SLOT(connCreated()));
}

MythFrame::~MythFrame() {
	m_mythLcd->deleteLater();
	m_server->close();
	m_server->deleteLater();;
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
        qDebug() << __PRETTY_FUNCTION__ << ": Setting timeout to 1000000 and will retry to set the acutal timeout then";
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
	QState *disconnected = new QState();

	disconnected->addTransition(this, SIGNAL(toConnectedState()), primary);
	primary->addTransition(this, SIGNAL(lcdDisconnect()), disconnected);
	metadata->addTransition(this, SIGNAL(lcdDisconnect()), disconnected);
	metadata->addTransition(this, SIGNAL(videoPlaybackEnded()), primary);
	nye->addTransition(this, SIGNAL(lcdDisconnect()), disconnected);
	nye->addTransition(this, SIGNAL(nyeEventDone()), primary);
	primary->addTransition(this, SIGNAL(videoPlaybackStarted()), metadata);
	primary->addTransition(this, SIGNAL(startNYE()), nye);
	metadata->addTransition(this, SIGNAL(startNYE()), nye);
	disconnected->addTransition(this, SIGNAL(startNYE()), nye);

	connect(disconnected, SIGNAL(entered()), this, SLOT(disconnectClock()));
	connect(metadata, SIGNAL(entered()), this, SLOT(metaDataStarted()));
	connect(metadata, SIGNAL(exited()), this, SLOT(hideMetadataScreen()));
	connect(primary, SIGNAL(exited()), this, SLOT(hidePrimaryScreen()));
	connect(nye, SIGNAL(exited()), this, SLOT(hideNYEScreen()));
	connect(metadata, SIGNAL(entered()), this, SLOT(showMetadataScreen()));
	connect(primary, SIGNAL(entered()), this, SLOT(showPrimaryScreen()));
	connect(nye, SIGNAL(entered()), this, SLOT(showNYEScreen()));

	m_states.addState(primary);
	m_states.addState(metadata);
	m_states.addState(nye);
	m_states.addState(disconnected);
	m_states.setInitialState(disconnected);

    showPrimaryScreen();
    hideNYEScreen();
    hideMetadataScreen();
    
    setNYETimeout();

    m_states.start();
    
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

	QString smallDisplay("<font style='font-size:80px; color:white; font-weight: bold;'>%1</font><br><font style='font-size:40px; color:gray;'>%2</font>");
	QString dateDisplay("<font style='font-size:50px; color:%1; font-weight: bold;'>%2</font>");
	QString largeDisplay("<font style='font-size:140px; color:%1; font-weight: bold;'>%2</font>");
	m_metaClock->setText(smallDisplay.arg(t.toString("h:mm a")).arg(d.toString()));
	m_primaryDate->setText(dateDisplay.arg(m_clockColor).arg(d.toString("dddd MMMM d, yyyy")));
	m_primaryClock->setText(largeDisplay.arg(m_clockColor).arg(t.toString("h:mm A")));
}

void MythFrame::connCreated()
{
	m_clockColor = "#FFFFFF";

	m_mythLcd = new LcdHandler(m_server->nextPendingConnection());

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
}

void MythFrame::mythConnected()
{
    emit toConnectedState();
}

void MythFrame::videoFormat(QString)
{
}

void MythFrame::audioFormat(QString v)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << v;
	if (v == "dts") {
		m_metaAudioImage->setStyleSheet(".QLabel{background-color: red; color: white; border-radius: 3px;}");
		QFont f("Liberation Sans", 20);
		m_metaAudioImage->setAlignment(Qt::AlignCenter);
		m_metaAudioImage->setFont(f);
		m_metaAudioImage->setText(v);
	}
	else {
		m_metaAudioImage->setText(v);
	}
}

void MythFrame::stereoFormat(QString f)
{
	m_metaStereoImage->setFont(QFont("Liberation Sans", 12));
	m_metaStereoImage->setAlignment(Qt::AlignCenter);

	qDebug() << __PRETTY_FUNCTION__ << ":" << f;
	if (f.compare("stereo", Qt::CaseInsensitive) == 0) {
		m_metaStereoImage->setText("Stereo");
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
	QPixmap hd("/usr/share/mythclock/HD.jpg");

	qDebug() << __PRETTY_FUNCTION__ << ":" << flags;
	if (flags == "Hi-Def") {
		if (hd.isNull())
			qDebug() << "Error opening hd pixmap";

		m_metaFlags->setPixmap(hd);
	}
	else {
		QFont f("Liberation Sans", 20);
		m_metaFlags->setFont(f);
		m_metaFlags->setText(flags);
	}
	m_metaFlags->setAlignment(Qt::AlignCenter);
}

void MythFrame::metaDataStarted()
{
	emit videoPlaybackStarted();
}

void MythFrame::metaDataEnded()
{
	emit videoPlaybackEnded();
}

void MythFrame::connClosed()
{
    qDebug() << __PRETTY_FUNCTION__;
	m_mythLcd->deleteLater();
	emit lcdDisconnect();
}

void MythFrame::channelUpdate(QByteArray s)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << s;
	m_metaChannel->setText(s.data());
}

void MythFrame::showTitle(QByteArray s)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << s;
	m_metaShow->setText(s.data());
}

void MythFrame::showSubTitle(QByteArray s)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << s;
	m_metaTitle->setText(s.data());
}

void MythFrame::elapsedTime(QByteArray ba)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << ba;
	m_metaTimeElapsed->setText(ba);
}

void MythFrame::totalTime(QByteArray ba)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << ba;
	prevTime = ba;
	m_metaTime->setText(ba);
}

void MythFrame::percentComplete(int p)
{
	qDebug() << __PRETTY_FUNCTION__ << ":" << p << "%";
	if (!bDisableProgress) {
		m_metaProgressBar->setValue(p);
	}
}

void MythFrame::showEvent(QShowEvent *e)
{
	Q_UNUSED(e)
    QFont c("Liberation Sans", 15);

	m_primaryClock->setGeometry(0, 0, 800, 320);
	m_primaryDate->setGeometry(0, 320, 800, 160);

	m_metaTitle->setGeometry(0, 0, width(), 200);
	m_metaShow->setGeometry(0, 200, 600, 80);
	m_metaChannel->setGeometry(0, 280, 600, 80);
	m_metaClock->setGeometry(0, 360, 600, 80);
	m_metaProgressBar->setGeometry(0, 440, width(), 40);

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
}

void MythFrame::hidePrimaryScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
	m_primaryClock->hide();
	m_primaryDate->hide();
}

void MythFrame::hideMetadataScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
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
    qDebug() << __PRETTY_FUNCTION__;
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
    qDebug() << __PRETTY_FUNCTION__;
	m_primaryClock->show();
	m_primaryDate->show();
}

void MythFrame::hideNYEScreen()
{
    qDebug() << __PRETTY_FUNCTION__;
	m_lbCountdown->hide();
}

void MythFrame::disconnectClock()
{
    m_clockColor = "#FFC266";
    showPrimaryScreen();
}
