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

void LcdHandler::messageAvailable()
{
	QByteArray ba = sock->readAll();
	qDebug() << ba;
}

void LcdHandler::closeConn()
{
	sock->close();
}
