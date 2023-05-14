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

#include "weatherdisplay.h"
#include "qmqttsubscriber.h"
#include "sonosdisplay.h"
#include "clockdisplay.h"
#include "bigclock.h"
#include "lux.h"

#define ONE_SECOND      1000
#define ONE_MINUTE      (ONE_SECOND * 60)
#define ONE_HOUR        (ONE_MINUTE * 60)
#define FIVE_HOURS      (ONE_HOUR * 60)

class PrimaryDisplay : public QMainWindow {
    Q_OBJECT
	
public:
    PrimaryDisplay();
    virtual ~PrimaryDisplay();

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void startNYE();
    void stopNYE();
    void startLightning();
    void endLightning();
//    void startBlankScreen();
//    void hideBlankScreen();
    void startWeather();
    void stopWeather();
    void hideWeather(bool);
    void hideSonos(bool);
    void hidePrimary(bool);
    void hideWeatherScreen();
    void dimDisplay();

protected slots:
    void showNYECountDown();
	void showPrimaryScreen();
	void showMetadataScreen();
	void showNYEScreen();
    void setNYETimeout();
    void showBlankScreen();
    void messageReceivedOnTopic(QString, QString);
    void connectionComplete();
    void disconnectedEvent();
    void lightningTimeout();
    void endMetadataScreen();
    void endWeatherScreen();
    void endBlankScreen();
    void showWeatherScreen();
    void updateNYEClock();
    void showDimScreen();
    void endDimScreen();
    void lux(long l);
    void setBacklight(bool state, uint8_t brightness);
    void showBigClock();
    void endBigClock();
    
private:
    typedef enum WIDGET_INDEX:int {
        Primary = 0,
        Metadata,
        NYE,
        Blank,
        Weather,
        Bigclock,
    } WidgetIndex;

    void setupMqttSubscriber();
    int getNightScreenTransitionTime();
    void enableBacklight(bool state, uint8_t brightness = 255);
    
    long myMap(long x, long in_min, long in_max, long out_min, long out_max)
    {
        long out = out_max - out_min;
        long in = (in_max - in_min) + out_min;
        if (in > 0) {
            return (x - in_min) * out / in;
        }
        return 255;
    }

    QMqttSubscriber *m_mqttClient;

    QWidget *m_primaryLayoutWidget;
    QWidget *m_nyeLayoutWidget;
    SonosDisplay *m_sonosWidget;
    QWidget *m_blankLayoutWidget;
    WeatherDisplay *m_weatherWidget;
    ClockDisplay *m_clockWidget;
    BigClock *m_bigClock;
    Lux *m_lux;

    QStackedWidget *m_stackedWidget;
    
    QGridLayout *m_primaryLayout;
    QHBoxLayout *m_nyeLayout;
    QGridLayout *m_sonosLayout;
   
	QByteArray prevTime;
    QTimer *m_startBlankScreen;
    QTimer *m_endBlankScreen;
    QTimer *m_endDimScreen;
    QTimer *m_endWeatherScreen;
    QTimer *m_startBigClockScreen;
    QTimer *m_endBigClockScreen;

    QLabel *m_lbCountdown;

    QStateMachine m_states;

    bool m_showBigClock;

    long m_lastBrightValue;
};

#endif /* MYTHFRAME_H_ */
