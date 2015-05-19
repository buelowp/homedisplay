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
#include <QtGui>
#include <QtWidgets>
#include <QtDebug>

#define MYTH_FLAG_SPEAKER_MASK		0x30
#define MYTH_FLAG_SPEAKER_STEREO	(1 << 4)
#define MYTH_FLAG_SPEAKER_5_1		(2 << 4)
#define MYTH_FLAG_SPEAKER_7_1		(3 << 4)

#define MYTH_AUDIO_FORMAT_MASK		0x70000
#define MYTH_AUDIO_FORMAT_MPEG2		(1 << 16)
#define MYTH_AUDIO_FORMAT_AC3		(2 << 16)
#define MYTH_AUDIO_FORMAT_DTS		(3 << 16)
#define MYTH_AUDIO_FORMAT_WMA		(4 << 16)

class LcdHandler : public QObject {
	Q_OBJECT
public:
	LcdHandler(QTcpSocket *sock);
	LcdHandler();
	virtual ~LcdHandler();

	void addSocket(QTcpSocket *sock);
	void closeConn();
	void ackSuccess();

signals:
	void sockClosed();
	void progressBarUpdate(int);
	void channelString(QString);
	void metaDataEnded();
	void videoFormat(QString);
	void audioFormat(QString);
	void stereoFormat(QString);
	void playbackFlags(QString);

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
