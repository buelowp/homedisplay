#pragma once

#include <noson/sonossystem.h>
#include <noson/contentdirectory.h>
#include <noson/avtransport.h>
#include <noson/musicservices.h>
#include <noson/smapi.h>
#include <noson/didlparser.h>

#include <QtCore/QtCore>
#include <QtStateMachine/QtStateMachine>

class Noson : public QObject
{
    Q_OBJECT
public:
    Noson(QObject *parent = nullptr);
    ~Noson() {};

    void go();

signals:
    void stop();
    void play();
    void startDisplay();
    void endDisplay();
    void title(QString t);
    void album(QString a);
    void artist(QString a);
    void duration(QTime t);
    void position(QTime t);

public slots:
    void switchState();
    void playerStopped();
    void playerStarted();

private:
    void setDuration(QString s);
    void parsePositionInfo();

    SONOS::System *m_sonos;
    SONOS::ZonePtr m_zone;
    SONOS::ZonePlayerList m_zonePlayer;
    SONOS::PlayerPtr m_player;
    QString m_playerLocation;
    QStateMachine *m_sm;
    QTime m_duration;
    QTime m_position;
};
