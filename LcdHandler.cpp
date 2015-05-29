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

#include "LcdHandler.h"

LcdHandler::LcdHandler()
{
	sock = NULL;
}

LcdHandler::LcdHandler(QTcpSocket *s) {
	if (s) {
		sock = s;
		connect(sock, SIGNAL(disconnected()), this, SLOT(disconnected()));
		connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
		connect(sock, SIGNAL(readyRead()), this, SLOT(messageAvailable()));
	}
	lcdState = 0;
}

LcdHandler::~LcdHandler()
{
	if (sock)
		delete sock;
}

void LcdHandler::disconnected()
{
	delete sock;
	sock = NULL;
	emit sockClosed();
}

void LcdHandler::error(QAbstractSocket::SocketError e)
{
	qDebug() << "LcdHandler:" << sock->errorString();
}

void LcdHandler::sendConnect()
{
	if (sock->state() == QAbstractSocket::ConnectedState) {
		sock->write("connect LCDproc 0.5.6 protocol 0.3 lcd wid 64 hgt 2 cellwid 1 cellhgt 8\n");
		lcdState++;
	}
}

void LcdHandler::setWidget(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');

	emit metaDataStarted();

	if (list[1] == "Channel") {
		if (list[2] == "topWidget") {
			ba.resize(ba.size() - 1);
			int pos = ba.indexOf('"');
			QString val = ba.mid(pos + 1, (ba.size() - (pos + 1)));
			emit channelString(val.trimmed());
		}
		if (list[2] == "progressBar") {
			emit progressBarUpdate(list[5].toInt());
		}
	}
}

void LcdHandler::setName(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	if (list[1] == "name") {
		QByteArray val = list[2];
		name = val;
	}
}

void LcdHandler::addScreen(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	QByteArray screen = list[1];
}

void LcdHandler::setOutput(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	QByteArray bitmap = list[1];

	if (bitmap == "0") {
		emit metaDataEnded();
	}
	else {
		int x = bitmap.toInt();

		if (x & 0x80000)
			emit videoFormat("mpg");
		else if (x & 0x100000)
			emit videoFormat("divx");
		else if (x & 0x180000)
			emit videoFormat("xvid");
		else if (x & 0x200000)
			emit videoFormat("wmv");

		if (x & 0x10000)
			emit audioFormat("mpeg2");
		if (x & 0x20000)
			emit audioFormat("ac3");
		if (x & 0x30000)
			emit audioFormat("dts");
		if (x & 0x40000)
			emit audioFormat("wma");

		if (x & 0x10)
			emit stereoFormat("stereo");
		else if (x & 0x20)
			emit stereoFormat("5.1");
		else if (x & 0x30)
			emit stereoFormat("7.1");

		if (x & 0x400)
			emit playbackFlags("Hi-Def");
	}
}

void LcdHandler::messageAvailable()
{
	QList<QByteArray> lines;
	QList<QByteArray>::iterator i;

	while (1) {
		QByteArray ba = sock->readLine();
		if (ba.size() == 0)
			break;
		else {
			ba.resize(ba.size() - 1);
			lines.push_back(ba);
		}
	}

	for (i = lines.begin(); i != lines.end(); i++) {
		QByteArray ba = *i;
		if (ba.size() == 0)
			continue;

		if (ba.left(5) == "hello") {
			sendConnect();
			continue;
		}
		// We don't care to store this, but it tells us LCDd thinks we're in good shape
		// Increment state to show we know we're good
		if (ba.contains("client_set")) {
			setName(ba);
			ackSuccess();
			continue;
		}
		if (ba.contains("widget_set")) {
			setWidget(ba);
			ackSuccess();
			continue;
		}
		if (ba.contains("widget_add")) {
			ackSuccess();
			continue;
		}
		if (ba.contains("screen_add")) {
			addScreen(ba);
			ackSuccess();
			continue;
		}
		if (ba.contains("screen_set")) {
			ackSuccess();
			continue;
		}
		if (ba.contains("output")) {
			setOutput(ba);
			ackSuccess();
			continue;
		}
	}
}

void LcdHandler::ackSuccess()
{
	sock->write("success");
}

void LcdHandler::closeConn()
{
	if (sock)
		sock->close();
}
