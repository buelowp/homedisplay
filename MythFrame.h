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
#include "sonoslabel.h"

#define ONE_SECOND      1000
#define ONE_MINUTE      (ONE_SECOND * 60)
#define ONE_HOUR        (ONE_MINUTE * 60)

class MythFrame : public QMainWindow {
    Q_OBJECT
	
public:
    MythFrame();
    virtual ~MythFrame();

signals:
    void startNYE();
    void stopNYE();
    void startLightning();
    void endLightning();
    void startSonos();
    void endSonos();
    void startBlankScreen();
    void endBlankScreen();

public slots:
    void updateClock();

protected slots:
    void showNYECountDown();
	void showPrimaryScreen();
	void showMetadataScreen();
	void showNYEScreen();
    void setNYETimeout();
//    void goDark();
//    void goPrimary();
    void showBlankScreen();
    void messageReceivedOnTopic(QString, QString);
    void connectionComplete();
    void disconnectedEvent();
    void lightningTimeout();
    void sonosRequestResult(QByteArray);
    void sonosRequestError(QNetworkReply::NetworkError);
    void sonosUpdate();
    void sonosAlbumArt(QByteArray);
    void sonosAlbumArtError(QNetworkReply::NetworkError);

private:
    
    typedef enum WIDGET_INDEX:int {
        Primary = 0,
        Sonos,
        NYE,
        Blank
    } WidgetIndex;
    
    typedef enum FONT_SIZE:int {
        Lightning = 28,
        Default = 36,
        Title = 50,
        Clock = 90,
    } FontSize;

    void setupMqttSubscriber();
    void setupSonos();
    void calculateMinutes(int);
    void setupBlankScreenTimers();
    
    QMqttSubscriber *m_mqttClient;
    SonosRequest *m_sonos;

    QWidget *m_primaryLayoutWidget;
    QWidget *m_nyeLayoutWidget;
    QWidget *m_sonosLayoutWidget;
    QWidget *m_blankLayoutWidget;

    QStackedWidget *m_stackedWidget;
    
    QGridLayout *m_primaryLayout;
    QGridLayout *m_nyeLayout;
    QGridLayout *m_sonosLayout;

	QLabel *m_primaryClock;
	SonosLabel *m_primaryDate;
	QLabel *m_lbCountdown;
	QTimer *m_clockTimer;
    QLabel *m_lightningLabel;
    QLabel *m_temperature;
    QLabel *m_humidity;
    
    SonosLabel *m_artist;
    SonosLabel *m_album;
    SonosLabel *m_station;
    SonosLabel *m_title;
    QLabel *m_albumArt;
    int m_duration;
    int m_elapsed;
    int m_trackNumber;
    int m_volume;
    QProgressBar *m_elapsedIndicator;
    
	QByteArray prevTime;
    QTimer *m_lightningTimer;
    QTimer *m_sonosTimer;
    QTimer *m_startBlankScreen;
    QTimer *m_endBlankScreen;

	QString m_clockColor;

	QStateMachine m_states;

    QVector<QLabel*> m_labels;
    WidgetIndex m_currentWidget;
};

#endif /* MYTHFRAME_H_ */
