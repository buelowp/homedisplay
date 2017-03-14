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

#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "MythClock.h"
#include "LcdHandler.h"

class MythFrame : public QFrame {
	Q_OBJECT
public:
	MythFrame(QFrame *parent = 0);
	virtual ~MythFrame();
	bool init();
	void startMetaData(bool);

signals:
	void lcdDisconnect();
	void videoPlaybackStarted();
	void videoPlaybackEnded();
	void startNYE();
	void stopNYE();

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
	void switchToVideoPlayback();
	void switchToPrimaryDisplay();

protected slots:
	void channelUpdate(QByteArray);
	void showTitle(QByteArray);
	void showSubTitle(QByteArray);
	void elapsedTime(QByteArray);
	void totalTime(QByteArray);
	void percentComplete(int);
	void showNYECountDown();
	void hidePrimaryScreen();
	void showPrimaryScreen();
	void hideMetadataScreen();
	void showMetadataScreen();
	void showNYEScreen();
	void hideNYEScreen();

protected:
	void showEvent(QShowEvent*);

private:

	QLabel *m_primaryClock;
	QTcpServer *m_server;
	LcdHandler *m_mythLcd;
	QLabel *m_metaChannel;
	QLabel *m_metaTitle;
	QLabel *m_metaShow;
	QLabel *m_metaAudioImage;
	QLabel *m_metaStereoImage;
	QLabel *m_metaFlags;
	QLabel *m_metaTime;
	QLabel *m_metaTimeElapsed;
	QLabel *m_metaClock;
	QLabel *m_primaryDate;
	QLabel *m_lbCountdown;
	QTimer *m_clockTimer;
	QByteArray prevTime;
	bool bDisableProgress;

	QProgressBar *m_metaProgressBar;
	QString m_clockColor;

	QStateMachine m_states;
};

#endif /* MYTHFRAME_H_ */
