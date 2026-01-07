#include "noson.h"

static void eventcb(void *param)
{
    if (param != nullptr) {
        Noson *noson = static_cast<Noson*>(param);
    }
}

static void playerEventcb(void *param)
{
    if (param != nullptr) {
        Noson *noson = static_cast<Noson*>(param);
        noson->switchState();
    }
}

Noson::Noson(QObject *parent) : QObject(parent)
{
    m_sonos = new SONOS::System(static_cast<void*>(this), eventcb);

    m_sm = new QStateMachine(this);
    QState *stopped = new QState();
    QState *playing = new QState();

    stopped->addTransition(this, &Noson::play, playing);
    playing->addTransition(this, &Noson::stop, stopped);

    connect(stopped, &QState::entered, this, &Noson::playerStopped);
    connect(playing, &QState::entered, this, &Noson::playerStarted);

    m_sm->addState(stopped);
    m_sm->addState(playing);
    m_sm->setInitialState(stopped);
    m_sm->start();
}

void Noson::playerStopped()
{
    qDebug() << __PRETTY_FUNCTION__;
    emit endDisplay();
}

void Noson::playerStarted()
{
    qDebug() << __PRETTY_FUNCTION__;
    emit startDisplay();
}

void Noson::parsePositionInfo()
{
    if (m_player->IsValid()) {
        SONOS::ElementList vars;
        m_player->GetPositionInfo(vars);
        for(int i = 0; i < vars.size(); ++i) {
            if (vars[i]->GetKey().c_str() == QString("RelTime")) {
                m_position = QTime::fromString(QString(vars[i]->c_str()), "h:mm:ss");
                emit position(m_position);
            }
            if (vars[i]->GetKey().c_str() == QString("TrackDuration")) {
                m_duration = QTime::fromString(QString(vars[i]->c_str()), "h:mm:ss");
                emit duration(m_duration);
            }
        }
    }
}

void Noson::switchState()
{
    QString state;
    SONOS::ElementList vars;

    if (m_player->IsValid()) {
        if (!m_player->TransportPropertyEmpty()) {
            emit title(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("dc:title").c_str()));
            emit album(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("upnp:album").c_str()));
            emit artist(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("dc:creator").c_str()));
            QString state = m_player->GetTransportProperty().TransportState.c_str();
            if (state == "PLAYING") {
                emit play();
            }
            else {
                emit stop();
            }
        }
        parsePositionInfo();
    }
    else {
        qWarning() << __PRETTY_FUNCTION__ << ": Player pointer was not valid, ignoring";
    }
}

void Noson::go()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");
    QString zone;
    if (settings.contains("zone")) {
        zone = settings.value("zone").toString();
        qDebug() << __PRETTY_FUNCTION__ << ":" << zone;
    }
    else {
        qWarning() << __PRETTY_FUNCTION__ << ": Unable to find zone key in settings";
        return;
    }

    if (m_sonos->Discover()) {
        SONOS::ZoneList zones = m_sonos->GetZoneList();
        for (SONOS::ZoneList::const_iterator it = zones.begin(); it != zones.end(); ++it) {
            QString zn = QString::fromUtf8(it->second->GetZoneName().c_str());
            if (zn == zone) {
                m_zone = it->second;
                qDebug() << __PRETTY_FUNCTION__ << ": found" << zn;
                break;
            }
        }
        if (QString::fromUtf8(m_zone->GetZoneName()).size() == 0) {
            qWarning() << __PRETTY_FUNCTION__ << ": Error finding" << zone << "zone, exiting";
            return;
        }
        SONOS::ZonePlayerList players = m_sonos->GetZonePlayerList();
        for (SONOS::ZonePlayerList::const_iterator it = players.begin(); it != players.end(); ++it) {
            QString zpl = QString::fromUtf8(it->second->c_str());
            if (zpl == zone) {
                m_playerLocation = QString::fromUtf8(it->second->GetLocation().c_str());
                qDebug() << __PRETTY_FUNCTION__ << ": found" << m_playerLocation;
                break;
            }
        }
        if (m_playerLocation.size() == 0) {
            qWarning() << __PRETTY_FUNCTION__ << ": Error finding" << zone << "player, exiting";
            return;
        }
        if (m_sonos->IsConnected()) {
            m_player = m_sonos->GetPlayer(m_zone, static_cast<void*>(this), playerEventcb);
            switchState();
        }
    }
    else {
        qWarning() << __PRETTY_FUNCTION__ << ": Unable to call Sonos Discover routine, exiting";
        return;
    }


    while (true) {
        QThread::sleep(1);
//        switchState();
    }
}
