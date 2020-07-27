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
#include <QtQmqtt/QtQmqtt>

#include "sonosrequest.h"
#include "qmqttsubscriber.h"

class MythFrame : public QFrame {
	Q_OBJECT
	
public:
	MythFrame(QFrame *parent = 0);
	virtual ~MythFrame();

signals:
	void startNYE();
	void stopNYE();
    void endLightning();
    void startLightning();
    void startSonos();
    void endSonos();

public slots:
    void updateClock();

protected slots:
    void showNYECountDown();
	void hidePrimaryScreen();
	void showPrimaryScreen();
	void hideMetadataScreen();
	void showMetadataScreen();
	void showNYEScreen();
	void hideNYEScreen();
    void setNYETimeout();
    void messageReceivedOnTopic(QString, QString);
    void connectionComplete();
    void disconnectedEvent();
    void lightningTimeout();
    void sonosRequestResult(QByteArray);
    void sonosRequestError(QNetworkReply::NetworkError);
    void sonosUpdate();
    void sonosAlbumArt(QByteArray);

protected:
	void showEvent(QShowEvent*);

private:
    
    typedef enum WIDGET_INDEX:int {
        Primary = 0,
        Sonos,
        NYE,
    } WidgetIndex;
    
    void setupMqttSubscriber();
    void setupSonos();
    void calculateMinutes(int);
    
    QMqttSubscriber *m_mqttClient;
    SonosRequest *m_sonos;

    QWidget *m_primaryLayoutWidget;
    QWidget *m_nyeLayoutWidget;
    QWidget *m_sonosLayoutWidget;

    QStackedLayout *m_stackedLayout;
    QVBoxLayout *m_parentLayout;
    
    QGridLayout *m_primaryLayout;
    QGridLayout *m_nyeLayout;
    QGridLayout *m_sonosLayout;

	QLabel *m_primaryClock;
	QLabel *m_primaryDate;
	QLabel *m_lbCountdown;
	QTimer *m_clockTimer;
    QLabel *m_lightningLabel;
    QLabel *m_temperature;
    QLabel *m_humidity;
    
    QLabel *m_artist;
    QLabel *m_album;
    QLabel *m_station;
    QLabel *m_albumArt;
    QLabel *m_title;
    int m_duration;
    int m_elapsed;
    int m_trackNumber;
    int m_volume;
    QProgressBar *m_elapsedIndicator;
    
	QByteArray prevTime;
    QTimer *m_lightningTimer;
    QTimer *m_sonosTimer;

	QString m_clockColor;

	QStateMachine m_states;
};

#endif /* MYTHFRAME_H_ */
