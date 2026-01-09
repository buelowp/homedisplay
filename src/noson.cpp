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

    m_updateTimer = new QTimer();
    connect(m_updateTimer, &QTimer::timeout, this, &Noson::switchState);
    m_updateTimer->setInterval(1000);

    m_namAlbumArt = new QNetworkAccessManager(this);
    connect(m_namAlbumArt, &QNetworkAccessManager::finished, this, &Noson::albumArtFinished);

    m_sm = new QStateMachine();
    QState *stopped = new QState();
    QState *playing = new QState();

    stopped->addTransition(this, &Noson::play, playing);
    playing->addTransition(this, &Noson::stop, stopped);

    connect(stopped, &QState::entered, this, &Noson::playerStopped);
    connect(playing, &QState::entered, this, &Noson::playerStarted);
    connect(m_sm, &QStateMachine::started, this, &Noson::smstart);
    connect(m_sm, &QStateMachine::stopped, this, &Noson::smstop);
    connect(m_sm, &QStateMachine::runningChanged, this, &Noson::smchange);
    connect(this, &Noson::getArt, this, &Noson::downloadAlbumArt);

    m_sm->addState(stopped);
    m_sm->addState(playing);
    m_sm->setInitialState(stopped);

    m_sm->start();
    qDebug() << __PRETTY_FUNCTION__ << ":" << m_sm->errorString() << ", is running" << m_sm->isRunning();
}

void Noson::smstart()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void Noson::smstop()
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << m_sm->errorString();
}

void Noson::smchange(bool state)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << state << ":" << m_sm->errorString();
}

void Noson::playerStopped()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_updateTimer->setInterval(1000);
    emit endDisplay();
}

void Noson::playerStarted()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_updateTimer->setInterval(100);
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

void Noson::downloadAlbumArt()
{
    QNetworkRequest request;
    request.setUrl(m_albumArtURL);
    request.setRawHeader("User-Agent", "SonosRequest 1.0");
    m_namAlbumArt->get(request);
}

void Noson::updateAlbumArt(QString aa)
{
    if (m_albumArtURI != aa) {
        m_albumArtURI = aa;
        QString local = m_albumArtURI.remove("amp;");
        QString full = QString("http://%1:%2%3").arg(QString::fromUtf8(m_player->GetHost())).arg(m_player->GetPort()).arg(local);
        m_albumArtURL.setUrl(full, QUrl::StrictMode);
        qDebug() << __PRETTY_FUNCTION__ << ":" << m_albumArtURL;
        emit getArt();
    }
}

void Noson::albumArtFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << __PRETTY_FUNCTION__ << ":" << reply->error();
    }
    else {
        QByteArray ba = reply->readAll();
        m_albumArt.loadFromData(ba);
        if (!m_albumArt.isNull()) {
            emit art(m_albumArt);
        }
    }
    reply->deleteLater();
}

//  "albumArtURI": "/getaa?s=1&u=x-sonos-spotify%3aspotify%253atrack%253a5OQGeJ1ceykovrykZsGhqL%3fsid%3d9%26flags%3d8224%26sn%3d3",
//<upnp:albumArtURI>/getaa?s=1&amp;u=x-sonosapi-hls-static%3aALkSOiHN75r4E3_3ofP6n_MzYeLvHTZxPfizDYorXqYeJWrm%3fsid%3d284%26flags%3d8%26sn%3d2</upnp:albumArtURI>
void Noson::switchState()
{
    QString state;
    SONOS::ElementList vars;

    if (m_player->IsValid()) {
        if (!m_player->TransportPropertyEmpty()) {
            emit title(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("dc:title").c_str()));
            emit album(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("upnp:album").c_str()));
            emit artist(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("dc:creator").c_str()));
            updateAlbumArt(QString(m_player->GetTransportProperty().CurrentTrackMetaData->GetValue("upnp:albumArtURI").c_str()));
            QString state = m_player->GetTransportProperty().TransportState.c_str();
            if (state == "PLAYING" || state == "TRANSITIONING") {
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
                qDebug() << __PRETTY_FUNCTION__ << ": found zone" << zn;
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
                qDebug() << __PRETTY_FUNCTION__ << ": talking to" << m_playerLocation;
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
        m_updateTimer->start();
    }
    else {
        qWarning() << __PRETTY_FUNCTION__ << ": Unable to call Sonos Discover routine, exiting";
        return;
    }
}
