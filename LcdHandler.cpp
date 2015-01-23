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

LcdHandler::LcdHandler() {
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
	QList<QByteArray> list;
	QList<QByteArray>::iterator i;

	if (ba.contains("Channel") && ba.contains("topWidget")) {
		int pos = ba.indexOf('"');
		if (pos != -1) {
			QByteArray metaData = ba.mid(pos + 1, ba.size() - pos);
			metaData.resize(metaData.size() - 2);
			emit enableChannelMeta(true);
			emit channelString(metaData.trimmed());
		}
		return;
	}
	else {
		list = ba.split(' ');
	}
	i = list.begin();
	i++;

	if (list[1] == "Time") {
		return;
	}
	if (list[1] == "Channel") {
		if (list[2] == "progressBar") {
			QByteArray pct = list[5].left(list[5].size() - 1);
			emit enableProgressBar(true);
			emit progressBarUpdate(pct.toInt());
		}
		return;
	}
}

void LcdHandler::setName(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	if (list[1] == "name") {
		QByteArray val = list[2];
		val.resize(val.size() - 1);
		name = val;
	}
}

void LcdHandler::addScreen(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	QByteArray screen = list[1];
	screen.resize(screen.size() - 1);
	qDebug() << "Adding screen" << screen;
}

void LcdHandler::setOutput(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	QByteArray bitmap = list[1];
	bitmap.resize(bitmap.size() - 1);

	qDebug() << "bitmap is" << bitmap;
	if (bitmap == "0") {
		emit enableProgressBar(false);
		emit enableChannelMeta(false);
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
		else
			lines.push_back(ba);
	}

	for (i = lines.begin(); i != lines.end(); i++) {
		QByteArray ba = *i;
		if (ba.size() == 0)
			continue;

		if (ba.left(5) == "hello") {
			sendConnect();
			return;
		}
		// We don't care to store this, but it tells us LCDd thinks we're in good shape
		// Increment state to show we know we're good
		if (ba.contains("client_set")) {
			setName(ba);
			return;
		}
		if (ba.contains("widget_set")) {
			setWidget(ba);
			return;
		}
		if (ba.contains("widget_add")) {
			return;
		}
		if (ba.contains("screen_add")) {
			addScreen(ba);
			return;
		}
		if (ba.contains("screen_set")) {
			return;
		}
		if (ba.contains("output")) {
			setOutput(ba);
			return;
		}
		qDebug() << ba;
	}
}

void LcdHandler::closeConn()
{
	sock->close();
}
