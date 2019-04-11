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
    m_connectionTimer = new QTimer(this);
    m_metadataTimer = new QTimer(this);
    m_pingTimer = new QTimer(this);
    m_kodi = new QTcpSocket(this);
    
    connect(m_kodi, SIGNAL(connected()), this, SLOT(kodiConnected()));
    connect(m_kodi, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
    connect(m_kodi, SIGNAL(readyRead()), this, SLOT(kodiResponse()));
    connect(m_kodi, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(kodiError(QAbstractSocket::SocketError)));
    connect(m_pingTimer, SIGNAL(timeout()), this, SLOT(ping()));
}

KodiLcdServer::~KodiLcdServer()
{

}

void KodiLcdServer::ping()
{
    QJsonObject p;
    qint64 now = QDateTime::currentSecsSinceEpoch();
    
    if (now - m_lastPing < 10) {
        p["id"] = 1;
        p["jsonrpc"] = "2.0";
        p["method"] = "JSONRPC.Ping";
        
        m_kodi->write(QJsonDocument(p).toJson());
    }
    else {
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
    qDebug() << __PRETTY_FUNCTION__ << ": lost connection to Kodi";
    m_pingTimer->stop();
    QTimer::singleShot(ONE_MINUTE, this, SLOT(start));
}

void KodiLcdServer::kodiConnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Connected to Kodi";
    m_pingTimer->setInterval(ONE_SECOND);
    m_pingTimer->start();
}

void KodiLcdServer::kodiResponse()
{
    QByteArray ba = m_kodi->readAll();
    QJsonDocument response = QJsonDocument::fromJson(ba);
    
    if (response.isObject()) {
        QJsonObject o = response.object();
        
        if (o.contains("result")) {
            if (o["result"] == "pong") {
                m_lastPing = QDateTime::currentSecsSinceEpoch();
                qDebug() << __PRETTY_FUNCTION__ << ": pong";
            }
        }
    }
}

void KodiLcdServer::kodiError(QAbstractSocket::SocketError socketError)
{
    qDebug() << __PRETTY_FUNCTION__ << ": connection error:" << socketError;
}
