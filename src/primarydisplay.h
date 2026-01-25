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
#include <QtMqtt/QtMqtt>
#include <QtStateMachine/QtStateMachine>

#include "weatherdisplay.h"
#include "sonosdisplay.h"
#include "clockdisplay.h"
#include "bigclock.h"
#include "lux.h"
#include "environment.h"
#include "defines.h"
#include "noson.h"
#include "nye.h"

class PrimaryDisplay : public QMainWindow {
    Q_OBJECT

public:
    PrimaryDisplay(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~PrimaryDisplay();

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void startNYE();
    void stopNYE();
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
    void showSonosScreen();
    void showNYEScreen();
    void setNYETimeout();
    void showBlankScreen();
    void endSonosScreen();
    void endWeatherScreen();
    void endBlankScreen();
    void showWeatherScreen();
    void showDimScreen();
    void endDimScreen();
    void lux(long l);
    void setBacklight(bool state, uint8_t brightness);
    void showBigClock();
    void endBigClock();
    void updateLocalConditions(double temp, double humidity);
    void connected();
    void disconnected();
    void errorChanged(QMqttClient::ClientError error);
    void messageReceived(const QByteArray &message, const QMqttTopicName &topic);
    bool event(QEvent *event) override;
    
private:
    typedef enum WIDGET_INDEX:int {
        Primary = 0,
        NYE,
        Blank,
        Weather,
        Bigclock,
        Sonos,
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

    QMqttClient *m_mqttClient;

    QWidget *m_primaryLayoutWidget;
    QWidget *m_nyeLayoutWidget;
    SonosDisplay *m_sonosWidget;
    QWidget *m_blankLayoutWidget;
    WeatherDisplay *m_weatherWidget;
    ClockDisplay *m_clockWidget;
    BigClock *m_bigClock;
    Lux *m_lux;
    Environment *m_environment;
    Noson *m_sonos;
    NYEWidget *m_nyeWidget;

    QStackedWidget *m_stackedWidget;
    
    QTimer *m_startBlankScreen;
    QTimer *m_endBlankScreen;
    QTimer *m_endDimScreen;
    QTimer *m_endWeatherScreen;
    QTimer *m_startBigClockScreen;
    QTimer *m_endBigClockScreen;
    QThread *m_sonosThread;

    QStateMachine m_states;

    bool m_showBigClock;

    long m_lastBrightValue;

    QString m_hostName;

    int m_maxBrightness;

    int m_width;
};

#endif /* MYTHFRAME_H_ */
