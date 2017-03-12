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

#ifndef MYTHFRAME_H_
#define MYTHFRAME_H_

#include <QtGui>
#include <QtNetwork>
#include <QtCore>
#include <QtWidgets>

#include "MythClock.h"
#include "LcdHandler.h"

class MythFrame : public QFrame {
	Q_OBJECT
public:
	MythFrame(QFrame *parent = 0);
	virtual ~MythFrame();
	bool init();
	void startMetaData(bool);

public slots:
	void connCreated();
	void connClosed();
	void metaDataEnded();
	void videoFormat(QString);
	void audioFormat(QString);
	void stereoFormat(QString);
	void playbackFlags(QString);
	void updateClock();
	void metaDataStarted();

protected slots:
	void channelUpdate(QByteArray);
	void showTitle(QByteArray);
	void showSubTitle(QByteArray);
	void elapsedTime(QByteArray);
	void totalTime(QByteArray);
	void percentComplete(int);

protected:
	void showEvent(QShowEvent*);

private:
	QLabel *digitalClock;
	QTcpServer *server;
	LcdHandler *conn;
	QLabel *channelLabel;
	QLabel *titleLabel;
	QLabel *showLabel;
	QLabel *audioIcon;
	QLabel *stereoIcon;
	QLabel *mythFlags;
	QLabel *lbTotalTime;
	QLabel *lbTimeElapsed;
	QLabel *lbClock;
	QLabel *m_lbDate;
	QTimer *pTimer;
	QByteArray prevTime;
	bool bDisableProgress;

	QProgressBar *pBar;
	QString clockColor;
};

#endif /* MYTHFRAME_H_ */
