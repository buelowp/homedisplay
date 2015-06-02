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

	digitalClock = new QLabel(this);
	server = new QTcpServer(this);
	channelLabel = new QLabel(this);
	titleLabel = new QLabel(this);
	showLabel = new QLabel(this);
	audioIcon = new QLabel(this);
	stereoIcon = new QLabel(this);
	mythFlags = new QLabel(this);
	lbClock = new QLabel(this);
	lbTimeElapsed = new QLabel(this);
	lbTotalTime = new QLabel(this);
	pBar = new QProgressBar(this);

	pTimer = new QTimer(this);
	connect(pTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
	pTimer->setInterval(500);
	pTimer->start();

	// Set the Myth connection indicator background black.
	channelLabel->setAutoFillBackground(true);
	channelLabel->setPalette(pal);
	titleLabel->setAutoFillBackground(true);
	titleLabel->setPalette(pal);
	showLabel->setAutoFillBackground(true);
	showLabel->setPalette(pal);
	audioIcon->setAutoFillBackground(true);
	audioIcon->setPalette(pal);
	stereoIcon->setAutoFillBackground(true);
	stereoIcon->setPalette(pal);
	mythFlags->setAutoFillBackground(true);
	mythFlags->setPalette(pal);
	lbClock->setAutoFillBackground(true);
	lbClock->setPalette(pal);


	pBar->setStyleSheet("QProgressBar {border: 0px solid black; border-radius: 0px; text-align: center; background-color: #000000;} QProgressBar::chunk {background-color: #00FFFF;}");

	pBar->setRange(0, 100);
	pBar->setTextVisible(false);
	conn = NULL;
	bDisableProgress = false;
}

MythFrame::~MythFrame() {
	delete conn;
	server->close();
	delete server;
}

bool MythFrame::init()
{
	connect(server, SIGNAL(newConnection()), this, SLOT(connCreated()));
	return server->listen(QHostAddress::Any, 6545);
}

void MythFrame::updateClock()
{
	QTime t = QTime::currentTime();
	QDate d = QDate::currentDate();

	QString smallDisplay("<font style='font-size:100px; color:white; font-weight: bold;'>%1</font><br><font style='font-size:40px; color:gray;'>%2</font>");
	QString largeDisplay("<font style='font-size:250px; color:white; font-weight: bold;'>%1</font>");
	lbClock->setText(smallDisplay.arg(t.toString("h:mm a")).arg(d.toString()));
	digitalClock->setText(smallDisplay.arg(t.toString("h:mm")));
}

void MythFrame::connCreated()
{
	if (conn == NULL) {
		conn = new LcdHandler(server->nextPendingConnection());
		connect(conn, SIGNAL(sockClosed()), this, SLOT(connClosed()));
		connect(conn, SIGNAL(channelNumber(QByteArray)), this, SLOT(channelUpdate(QByteArray)));
		connect(conn, SIGNAL(showTitle(QByteArray)), this, SLOT(showTitle(QByteArray)));
		connect(conn, SIGNAL(showSubTitle(QByteArray)), this, SLOT(showSubTitle(QByteArray)));
		connect(conn, SIGNAL(progressTimeLeft(QByteArray)), this, SLOT(elapsedTime(QByteArray)));
		connect(conn, SIGNAL(progressTotalTime(QByteArray)), this, SLOT(totalTime(QByteArray)));
		connect(conn, SIGNAL(progressPercentComplete(int)), this, SLOT(percentComplete(int)));
		connect(conn, SIGNAL(metaDataEnded()), this, SLOT(metaDataEnded()));
		connect(conn, SIGNAL(videoFormat(QString)), this, SLOT(videoFormat(QString)));
		connect(conn, SIGNAL(audioFormat(QString)), this, SLOT(audioFormat(QString)));
		connect(conn, SIGNAL(stereoFormat(QString)), this, SLOT(stereoFormat(QString)));
		connect(conn, SIGNAL(playbackFlags(QString)), this, SLOT(playbackFlags(QString)));
		connect(conn, SIGNAL(metaDataStarted()), this, SLOT(metaDataStarted()));
	}
}

void MythFrame::videoFormat(QString)
{
}

void MythFrame::audioFormat(QString v)
{
	if (v == "dts") {
		audioIcon->setStyleSheet(".QLabel{background-color: red; color: white; border-radius: 3px;}");
		QFont f("Liberation Sans", 20);
		audioIcon->setAlignment(Qt::AlignCenter);
		audioIcon->setFont(f);
		audioIcon->setText(v);
	}
	else {
		audioIcon->setText(v);
	}
}

void MythFrame::stereoFormat(QString f)
{
	stereoIcon->setFont(QFont("Liberation Sans", 12));
	stereoIcon->setAlignment(Qt::AlignCenter);

	if (f.compare("stereo", Qt::CaseInsensitive) == 0) {
		stereoIcon->setText("Stereo");
	}
	if (f == "5.1") {
		stereoIcon->setPixmap(QPixmap("/usr/share/mythclock/5_1.jpg"));
	}
	if (f == "7.1") {
		stereoIcon->setPixmap(QPixmap("/usr/share/mythclock/7_1.jpg"));
	}
}

void MythFrame::playbackFlags(QString flags)
{
	QPixmap hd("/usr/share/mythclock/HD.jpg");

	if (flags == "Hi-Def") {
		if (hd.isNull())
			qDebug() << "Error opening hd pixmap";

		mythFlags->setPixmap(hd);
	}
	else {
		QFont f("Liberation Sans", 20);
		mythFlags->setFont(f);
		mythFlags->setText(flags);
	}
	mythFlags->setAlignment(Qt::AlignCenter);
}

void MythFrame::metaDataStarted()
{
	startMetaData(true);
}

void MythFrame::metaDataEnded()
{
	showLabel->setText("");
	titleLabel->setText("");
	channelLabel->setText("");
	audioIcon->setText("");
	stereoIcon->setText("");
	mythFlags->setText("");
	lbTimeElapsed->setText("");
	lbTotalTime->setText("");
	pBar->reset();

	QPalette pal(QColor(0,0,0));
	audioIcon->setBackgroundRole(QPalette::Window);
	pal.setColor(QPalette::Window, Qt::black);
	audioIcon->setAutoFillBackground(true);
	audioIcon->setPalette(pal);
	startMetaData(false);
}

void MythFrame::connClosed()
{
	if (conn) {
		disconnect(conn, SIGNAL(sockClosed()), this, SLOT(connClosed()));
		disconnect(conn, SIGNAL(channelNumber(QByteArray)), this, SLOT(channelUpdate(QByteArray)));
		disconnect(conn, SIGNAL(showTitle(QByteArray)), this, SLOT(showTitle(QByteArray)));
		disconnect(conn, SIGNAL(showSubTitle(QByteArray)), this, SLOT(showSubTitle(QByteArray)));
		disconnect(conn, SIGNAL(progressTimeLeft(QByteArray)), this, SLOT(elapsedTime(QByteArray)));
		disconnect(conn, SIGNAL(progressTotalTime(QByteArray)), this, SLOT(totalTime(QByteArray)));
		disconnect(conn, SIGNAL(progressPercentComplete(int)), this, SLOT(percentComplete(int)));
		disconnect(conn, SIGNAL(metaDataEnded()), this, SLOT(metaDataEnded()));
		disconnect(conn, SIGNAL(videoFormat(QString)), this, SLOT(videoFormat(QString)));
		disconnect(conn, SIGNAL(audioFormat(QString)), this, SLOT(audioFormat(QString)));
		disconnect(conn, SIGNAL(stereoFormat(QString)), this, SLOT(stereoFormat(QString)));
		disconnect(conn, SIGNAL(playbackFlags(QString)), this, SLOT(playbackFlags(QString)));
		disconnect(conn, SIGNAL(metaDataStarted()), this, SLOT(metaDataStarted()));
		delete conn;
		conn = NULL;
	}
}

void MythFrame::channelUpdate(QByteArray s)
{
	channelLabel->setText(s.data());
}

void MythFrame::showTitle(QByteArray s)
{
	showLabel->setText(s.data());
}

void MythFrame::showSubTitle(QByteArray s)
{
	titleLabel->setText(s.data());
}

void MythFrame::elapsedTime(QByteArray ba)
{
	lbTimeElapsed->setStyleSheet(".QLabel{font-size:30px; background-color: black; color: white;}");
	lbTimeElapsed->setText(ba);
}

void MythFrame::totalTime(QByteArray ba)
{
	prevTime = ba;
	lbTotalTime->setStyleSheet(".QLabel{font-size:30px; background-color: black; color: white;}");
	lbTotalTime->setText(ba);
}

void MythFrame::percentComplete(int p)
{
	if (!bDisableProgress) {
		pBar->setValue(p);
	}
}

void MythFrame::startMetaData(bool event)
{
	if (event) {
		digitalClock->hide();
		lbClock->show();
		titleLabel->show();
		showLabel->show();
		channelLabel->show();
		audioIcon->show();
		stereoIcon->show();
		mythFlags->show();
		lbTimeElapsed->show();
		lbTotalTime->show();
		pBar->show();
	}
	else {
		digitalClock->show();
		lbClock->hide();
		titleLabel->hide();
		showLabel->hide();
		channelLabel->hide();
		audioIcon->hide();
		stereoIcon->hide();
		mythFlags->hide();
		lbTimeElapsed->hide();
		lbTotalTime->hide();
		pBar->hide();
	}
}

void MythFrame::showEvent(QShowEvent*)
{
	int clockWidth = 600;
	int clockHeight = 330;
	int titlePanelsHeight = 50;
	int iconPanelsWidth = 100;
	int iconPanelsHeight = 66;

	digitalClock->setGeometry(0, 0, width(), height());

	lbClock->setGeometry(0, 0, clockWidth, clockHeight);
	QFont clockFont("Liberation Sans");
	lbClock->setFont(clockFont);
	lbClock->setAlignment(Qt::AlignCenter);
	lbClock->show();

	QFont c("Liberation Sans", 15);
	titleLabel->setFont(c);
	showLabel->setFont(c);
	showLabel->setGeometry(0, lbClock->height(), width(), titlePanelsHeight);
	showLabel->setIndent(10);
	showLabel->show();
	titleLabel->setGeometry(0, lbClock->height() + titlePanelsHeight, width(), titlePanelsHeight);
	titleLabel->show();
	titleLabel->setIndent(10);
	titleLabel->setFont(c);
	channelLabel->setGeometry(0, lbClock->height() + (titlePanelsHeight * 2), width(), titlePanelsHeight);
	channelLabel->show();
	channelLabel->setIndent(10);
	channelLabel->setFont(c);

	audioIcon->setGeometry(clockWidth, 0, iconPanelsWidth, iconPanelsHeight);
	audioIcon->show();

	stereoIcon->setGeometry(clockWidth + iconPanelsWidth, 0, iconPanelsWidth, iconPanelsHeight);
	stereoIcon->show();

	mythFlags->setGeometry(clockWidth + iconPanelsWidth, iconPanelsHeight, iconPanelsWidth, iconPanelsHeight);
	mythFlags->show();

	lbTimeElapsed->setGeometry(clockWidth, iconPanelsHeight * 2, iconPanelsWidth * 2, iconPanelsHeight);
	lbTimeElapsed->setAlignment(Qt::AlignCenter);

	lbTotalTime->setGeometry(clockWidth, iconPanelsHeight * 3, iconPanelsWidth * 2, iconPanelsHeight);
	lbTotalTime->setAlignment(Qt::AlignCenter);

	pBar->setGeometry(clockWidth, iconPanelsHeight * 4, iconPanelsWidth * 2, iconPanelsHeight);
	pBar->setStyleSheet(".QProgressBar{border: 2px solid black; background: black; padding: 2px;} QProgressBar::chunk{border-radius: 3px; background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #fff, stop: .25 #fee, stop: .5 #fbb, stop: .75 #f66, stop: 1 #f00);}");

	startMetaData(false);
}
