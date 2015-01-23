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

	clock = new MythClock(this);
	server = new QTcpServer(this);
	mythConn = new QLabel(this);
	channelLabel = new QLabel(this);
	pBar = new QProgressBar(this);

	// Set the Myth connection indicator background black.
	mythConn->setAutoFillBackground(true);
	mythConn->setPalette(pal);
	mythConn->setAlignment(Qt::AlignRight);
	channelLabel->setAutoFillBackground(true);
	channelLabel->setPalette(pal);
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

void MythFrame::connCreated()
{
	if (conn == NULL) {
		conn = new LcdHandler(server->nextPendingConnection());
		connect(conn, SIGNAL(sockClosed()), this, SLOT(connClosed()));
		mythConn->setText("<font color='blue'>MythTV</font>");
		connect(conn, SIGNAL(channelString(QString)), this, SLOT(channelUpdate(QString)));
		connect(conn, SIGNAL(progressBarUpdate(int)), pBar, SLOT(setValue(int)));
		connect(conn, SIGNAL(enableProgressBar(bool)), this, SLOT(enableProgressBar(bool)));
		connect(conn, SIGNAL(enableChannelMeta(bool)), this, SLOT(enableChannelMeta(bool)));
	}
}

void MythFrame::enableProgressBar(bool v)
{
	if (v) {
		pBar->show();
	}
	else {
		pBar->hide();
	}
}

void MythFrame::enableChannelMeta(bool v)
{
	if (v)
		channelLabel->show();
	else {
		channelLabel->hide();
		chanMetaData.clear();
	}
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
	for (int i = 0; i < chanMetaData.size(); i++) {
		if (chanMetaData[i] == s) {
			return;
		}
	}
	chanMetaData.push_back(s);

	QString meta;
	for (int i = 0; i < chanMetaData.size(); i++) {
		meta += chanMetaData[i];
		meta += "\n";
	}
	channelLabel->setText(meta);
}

void MythFrame::showEvent(QShowEvent*)
{
	int middle = (width() - height()) / 2;
	clock->setGeometry(0, 0, height(), height());
	clock->show();
	mythConn->setText("<font color='gray'>MythTV</font>");
	mythConn->setGeometry(272, 0, 208, 10);
	QFont f("Times", 6);
	mythConn->setFont(f);

	channelLabel->setGeometry(272, 10, 208, 100);
	channelLabel->hide();
	QFont c("Liberation Sans", 10);
	channelLabel->setFont(c);

	pBar->setGeometry(272, 252, 208, 20);
	pBar->hide();
}
