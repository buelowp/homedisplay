/*
 * Copyright (c) 2019 <copyright holder> <email>
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "KodiLcdServer.h"

KodiLcdServer::KodiLcdServer(QObject *parent) : QObject(parent)
{
    m_connected = false;
    m_mediaPlaybackStarted = false;
    m_playerId = 0;
    
    m_connectionTimer = new QTimer(this);
    m_metadataTimer = new QTimer(this);
    m_pingTimer = new QTimer(this);
    m_mediaCheck = new QTimer(this);
    m_kodi = new QTcpSocket(this);
    
    connect(m_kodi, SIGNAL(connected()), this, SLOT(kodiConnected()));
    connect(m_kodi, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
    connect(m_kodi, SIGNAL(readyRead()), this, SLOT(kodiResponse()));
    connect(m_kodi, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(kodiError(QAbstractSocket::SocketError)));
    connect(m_pingTimer, SIGNAL(timeout()), this, SLOT(ping()));
    connect(m_mediaCheck, SIGNAL(timeout()), this, SLOT(testForPlayback()));
    connect(m_metadataTimer, SIGNAL(timeout()), this, SLOT(getPlaybackMetaData()));
}

KodiLcdServer::~KodiLcdServer()
{

}

void KodiLcdServer::ping()
{
    QJsonObject p;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    
    if (now - m_lastPing < 11000) {
        p["id"] = PING_ID;
        p["jsonrpc"] = "2.0";
        p["method"] = "JSONRPC.Ping";
        
        m_kodi->write(QJsonDocument(p).toJson(QJsonDocument::Compact));
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ": No Kodi PONG response in 10 seconds, closing connection";
        m_kodi->disconnectFromHost();
    }
}

void KodiLcdServer::exec()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    QString hostname = settings.value("kodiserver").toString();
    QHostInfo lookup = QHostInfo::fromName(hostname);
    QList<QHostAddress> addresses = lookup.addresses();

    if (addresses.size()) {
        qDebug() << __PRETTY_FUNCTION__ << ": Connecting to" << hostname << ":" << settings.value("kodiport").toInt();
        m_kodi->connectToHost(addresses.at(0), settings.value("kodiport").toInt());
    }
}

void KodiLcdServer::connectionClosed()
{
    m_connected = false;
    qDebug() << __PRETTY_FUNCTION__ << ": lost connection to Kodi, will retry in 60 seconds";
    m_pingTimer->stop();
    QTimer::singleShot(ONE_MINUTE, this, SLOT(start()));
}

void KodiLcdServer::kodiConnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Connected to Kodi";
    m_pingTimer->setInterval(ONE_SECOND);
    m_pingTimer->start();
    m_connected = true;
    emit clientConnected();
}

void KodiLcdServer::metaDataStartedEvent(QJsonObject json)
{
    QJsonObject data = json["data"].toObject();
    QJsonObject item = data["item"].toObject();
    QJsonObject player = data["player"].toObject();
    
    qDebug() << player;
    m_playerId = player["playerid"].toInt();
    
    emit showTitle(item["title"].toVariant().toByteArray());
    requestAudioMetaData();
    requestVideoMetaData();
}

void KodiLcdServer::playerData(QJsonArray&)
{
}

void KodiLcdServer::kodiResponse()
{
    QByteArray ba = m_kodi->readAll();
    QJsonDocument response = QJsonDocument::fromJson(ba);
    
    if (response.isObject()) {
        QJsonObject o = response.object();
        
        if (o.contains("method")) {
            if (o["method"] == "Player.OnPlay") {
                m_mediaPlaybackStarted = true;
                emit metaDataStarted();
                m_metadataTimer->setInterval(1000);
                m_metadataTimer->start();
                metaDataStartedEvent(o["params"].toObject());
            }
            else if (o["method"] == "Player.OnStop") {
                m_mediaPlaybackStarted = false;
                m_metadataTimer->stop();
                emit metaDataEnded();
            }
            else {
                qDebug() << ba;
            }
        }
        else {
            if (o["id"] == PING_ID) {
                if (o["result"] == "pong")
                    m_lastPing = QDateTime::currentMSecsSinceEpoch();
            }
            else if (o["id"] == INT_METADATA_ID) {
                parsePlaybackMetaData(o["result"].toObject());
            }
        }
    }
}

void KodiLcdServer::parsePlaybackMetaData(QJsonObject json)
{
    QJsonObject time = json["time"].toObject();
    QJsonObject totalTime = json["totaltime"].toObject();
    QJsonObject audioStream = json["currentaudiostream"].toObject();
    QJsonObject videoStream = json["currentvideostream"].toObject();
    int channels = audioStream["channels"].toInt();
    QString codec = audioStream["codec"].toString();
    int width = videoStream["width"].toInt();
    
    emit progressTotalTime(QString("%1:%2:%3").arg(totalTime["hours"].toInt()).arg(totalTime["minutes"].toInt()).arg(totalTime["seconds"].toInt()).toUtf8());
    emit progressTimeLeft(QString("%1:%2:%3").arg(time["hours"].toInt()).arg(time["minutes"].toInt()).arg(time["seconds"].toInt()).toUtf8());
    emit progressPercentComplete(json["percentage"].toDouble());
    if (codec.contains("dts"))
        emit audioFormat("dts");
    if (codec.contains("ac3"))
        emit audioFormat("ac3");
    
    if (width > 1800)
        emit playbackFlags("Hi-Def");
    
    switch (channels) {
        case 5:
            emit stereoFormat("5.1");
            break;
        case 7:
            emit stereoFormat("7.1");
            break;
        case 2:
        default:
            emit stereoFormat("stereo");
            break;
    }
}

void KodiLcdServer::kodiError(QAbstractSocket::SocketError socketError)
{
    qDebug() << __PRETTY_FUNCTION__ << ": connection error:" << socketError;
}

void KodiLcdServer::testForPlayback()
{
    QJsonObject test;
    
    if (m_connected) {
        test["id"] = PLAYER_ID;
        test["jsonrpc"] = "2.0";
        test["method"] = "Player.GetActivePlayers";
        
        m_kodi->write(QJsonDocument(test).toJson(QJsonDocument::Compact));
    }
}

void KodiLcdServer::getPlaybackMetaData()
{
    QJsonObject request;
    QJsonObject properties;
    QJsonArray params;
    
    if (m_connected) {
        params.append("percentage");
        params.append("time");
        params.append("totaltime");
        params.append("currentaudiostream");
        params.append("currentvideostream");
        
        properties["playerid"] = m_playerId;
        properties["properties"] = params;

        request["jsonrpc"] = "2.0";
        request["method"] = "Player.GetProperties";
        request["params"] = properties;
        request["id"] = INT_METADATA_ID;

        m_kodi->write(QJsonDocument(request).toJson(QJsonDocument::Compact));
    }
}

void KodiLcdServer::requestAudioMetaData()
{
    if (m_playerId) {
        
    }
}

void KodiLcdServer::requestPlaybackProperties()
{
}

void KodiLcdServer::requestVideoMetaData()
{
}
