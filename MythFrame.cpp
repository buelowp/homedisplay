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

//	clock = new MythClock(this);
	server = new QTcpServer(this);
	mythConn = new QLabel(this);
	channelLabel = new QLabel(this);
	titleLabel = new QLabel(this);
	showLabel = new QLabel(this);
	audioIcon = new QLabel(this);
	videoIcon = new QLabel(this);
	stereoIcon = new QLabel(this);
	mythFlags = new QLabel(this);
	lbClock = new QLabel(this);
	pBar = new QProgressBar(this);

	pTimer = new QTimer(this);
	connect(pTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
	pTimer->setInterval(1000);
	pTimer->start();

	// Set the Myth connection indicator background black.
	mythConn->setAutoFillBackground(true);
	mythConn->setPalette(pal);
	mythConn->setAlignment(Qt::AlignRight);
	channelLabel->setAutoFillBackground(true);
	channelLabel->setPalette(pal);
	titleLabel->setAutoFillBackground(true);
	titleLabel->setPalette(pal);
	showLabel->setAutoFillBackground(true);
	showLabel->setPalette(pal);
	audioIcon->setAutoFillBackground(true);
	audioIcon->setPalette(pal);
	videoIcon->setAutoFillBackground(true);
	videoIcon->setPalette(pal);
	stereoIcon->setAutoFillBackground(true);
	stereoIcon->setPalette(pal);
	mythFlags->setAutoFillBackground(true);
	mythFlags->setPalette(pal);
	lbClock->setAutoFillBackground(true);
	lbClock->setPalette(pal);


	pBar->setStyleSheet("QProgressBar {border: 0px solid black; border-radius: 0px; text-align: center; background-color: #000000;} QProgressBar::chunk {background-color: #00FFFF;}");

	pBar->setRange(0, 64);
	pBar->setTextVisible(false);
	conn = NULL;
}

MythFrame::~MythFrame() {
	conn->closeConn();
	delete conn;
	server->close();
	delete server;
}

bool MythFrame::init()
{
	connect(server, SIGNAL(newConnection()), this, SLOT(connCreated()));
	return server->listen(QHostAddress::LocalHost, 13666);
}

void MythFrame::updateClock()
{
	QTime t = QTime::currentTime();

	lbClock->setText(t.toString("h:mm a"));
}

void MythFrame::connCreated()
{
	if (conn == NULL) {
		conn = new LcdHandler(server->nextPendingConnection());
		connect(conn, SIGNAL(sockClosed()), this, SLOT(connClosed()));
		mythConn->setText("<font color='blue'>MythTV</font>");
		connect(conn, SIGNAL(channelString(QString)), this, SLOT(channelUpdate(QString)));
		connect(conn, SIGNAL(progressBarUpdate(int)), pBar, SLOT(setValue(int)));
		connect(conn, SIGNAL(metaDataEnded()), this, SLOT(metaDataEnded()));
		connect(conn, SIGNAL(videoFormat(QString)), this, SLOT(videoFormat(QString)));
		connect(conn, SIGNAL(audioFormat(QString)), this, SLOT(audioFormat(QString)));
		connect(conn, SIGNAL(stereoFormat(QString)), this, SLOT(stereoFormat(QString)));
		connect(conn, SIGNAL(playbackFlags(QString)), this, SLOT(playbackFlags(QString)));
	}
}

void MythFrame::videoFormat(QString v)
{
	videoIcon->setText(v);
}

void MythFrame::audioFormat(QString v)
{
	if (v == "dts") {
		QPalette pal(QColor(0,0,0));
		audioIcon->setBackgroundRole(QPalette::Window);
		pal.setColor(QPalette::Window, Qt::red);
		audioIcon->setAutoFillBackground(true);
		audioIcon->setPalette(pal);
		audioIcon->setText("<center><bold>DTS</bold></center>");
	}
	else {
		audioIcon->setText(v);
	}
}

void MythFrame::stereoFormat(QString f)
{
	if (f == "stereo") {
		stereoIcon->setText("Stereo");
//		stereoIcon->setPixmap(QPixmap("icons/stereo.jpg"));
	}
	if (f == "5.1") {
		stereoIcon->setText("5.1");
	}
	if (f == "7.1") {
		stereoIcon->setText("7.1");
	}
}

void MythFrame::playbackFlags(QString flags)
{
	mythFlags->setText(flags);
}

void MythFrame::metaDataEnded()
{
	showLabel->setText("");
	titleLabel->setText("");
	videoIcon->setText("");
	audioIcon->setText("");
	stereoIcon->setText("");
	mythFlags->setText("");
	pBar->reset();

	QPalette pal(QColor(0,0,0));
	audioIcon->setBackgroundRole(QPalette::Window);
	pal.setColor(QPalette::Window, Qt::black);
	audioIcon->setAutoFillBackground(true);
	audioIcon->setPalette(pal);
}

void MythFrame::connClosed()
{
	mythConn->setText("<font color='gray'>MythTV</font>");
	if (conn) {
		delete conn;
		conn = NULL;
	}
}

void MythFrame::channelUpdate(QString s)
{
	if (showLabel->text().size() == 0) {
		showLabel->setText(s);
		return;
	}
	// The movie conundrum
	if (showLabel->text() == s)
		return;

	if (titleLabel->text().size() == 0) {
		titleLabel->setText(s);
		return;
	}
	if (channelLabel->text().size() == 0) {
		channelLabel->setText(s);
		return;
	}
}

void MythFrame::showEvent(QShowEvent*)
{
	lbClock->setGeometry(0, 10, 380, 172);
	QFont clockFont("Liberation Sans", 60);
	lbClock->setFont(clockFont);
	lbClock->setAlignment(Qt::AlignCenter);
	lbClock->show();

	mythConn->setText("<font color='gray'>MythTV</font>");
	mythConn->setGeometry(0, 0, 480, 10);
	QFont f("Times", 6);
	mythConn->setFont(f);

	QFont c("Liberation Sans", 24);
	titleLabel->setFont(c);
	showLabel->setFont(c);
	showLabel->setGeometry(0, 172, 480, 50);
	showLabel->setIndent(10);
	showLabel->show();
	titleLabel->setGeometry(0, 222, 480, 50);
	titleLabel->show();
	titleLabel->setIndent(10);
	channelLabel->hide();

	audioIcon->setGeometry(380, 30, 50, 20);
	audioIcon->show();
	videoIcon->setGeometry(430, 30, 50, 20);
	videoIcon->show();
	stereoIcon->setGeometry(380, 50, 50, 20);
	stereoIcon->show();
	mythFlags->setGeometry(430, 50, 50, 20);
	mythFlags->show();

	pBar->setGeometry(380, 152, 100, 20);
	pBar->setValue(0);
	pBar->show();
}
