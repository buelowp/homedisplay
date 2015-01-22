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
	QPalette pal(palette());
	pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

	clock = new MythClock(this);
	server = new QTcpServer(this);
	mythConn = new QLabel(this);

	// Set the Myth connection indicator background black.
	mythConn->setAutoFillBackground(true);
	mythConn->setPalette(pal);
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

void MythFrame::showEvent(QShowEvent*)
{
	clock->resize(width(), height());
	clock->show();
	mythConn->setText("<font color='gray'>MythTV</font>");
	mythConn->setGeometry(450, 0, 30, 10);
	QFont f("Times", 6);
	mythConn->setFont(f);
}
