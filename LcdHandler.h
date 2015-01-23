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

#ifndef LCDHANDLER_H_
#define LCDHANDLER_H_

#include <QtCore>
#include <QtNetwork>

class LcdHandler : public QObject {
	Q_OBJECT
public:
	LcdHandler(QTcpSocket *sock);
	LcdHandler();
	virtual ~LcdHandler();

	void addSocket(QTcpSocket *sock);
	void closeConn();

signals:
	void sockClosed();
	void progressBarUpdate(int);
	void channelString(QString);
	void enableProgressBar(bool);
	void enableChannelMeta(bool);

public slots:
	void disconnected();
	void error(QAbstractSocket::SocketError);
	void messageAvailable();

private:
	void sendConnect();
	void setWidget(QByteArray&);
	void setName(QByteArray&);
	void addScreen(QByteArray&);
	void setOutput(QByteArray&);

	int lcdState;
	QTcpSocket *sock;
	QString name;
};

#endif /* LCDHANDLER_H_ */
