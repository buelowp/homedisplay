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

void LcdHandler::error(QAbstractSocket::SocketError)
{
	qDebug() << "LcdHandler:" << sock->errorString();
}

void LcdHandler::sendConnect()
{
	if (sock->state() == QAbstractSocket::ConnectedState) {
		sock->write("CONNECTED 64 2\n");
		lcdState++;
	}
}

void LcdHandler::setOutput(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');
	QByteArray bitmap = list[1];

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

void LcdHandler::setChannelProgress(QByteArray &ba)
{
	QList<QByteArray> list = ba.split(' ');

	QByteArray timeLeft = list[1];
	if (timeLeft.size() > 1) {
		emit progressTimeLeft(timeLeft.mid(1, timeLeft.size()));
	}

	QByteArray totalTime = list[3];
	if (totalTime.size() > 1) {
		totalTime.resize(totalTime.size() - 1);
		emit progressTotalTime(totalTime);
		QByteArray pcntComplete = list[4];
		int convert = pcntComplete.toDouble() * 100;
		emit progressPercentComplete(convert);
	}
}

void LcdHandler::setChannelData(QByteArray &ba)
{
	QList<QByteArray> list = ba.split('"');

	emit metaDataStarted();

	QByteArray chanNum = list[1];
	if (chanNum.size() > 2)
		emit channelNumber(chanNum);

	QByteArray title = list[3];
	if (title.size() > 2)
		emit showTitle(title);

	QByteArray sub = list[5];
	if (sub.size() > 3)
		emit showSubTitle(sub);
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

		if (ba.left(5) == "HELLO") {
			sendConnect();
			continue;
		}
		if (ba.contains("SWITCH_TO_CHANNEL")) {
			setChannelData(ba);
			continue;
		}
		if (ba.contains("SWITCH_TO_TIME")) {
			emit metaDataEnded();
			continue;
		}
		if (ba.contains("SET_CHANNEL_PROGRESS")) {
			setChannelProgress(ba);
			continue;
		}
		if (ba.contains("UPDATE_LEDS")) {
			setOutput(ba);
			continue;
		}
	}
}
