// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "qmqueue.h"

QMQueue::QMQueue(QObject *parent) : QObject(parent), m_open(false)
{
    QString conf("/home/pi/.config/MythClock/MythClock.ini");
    QSettings settings(conf, QSettings::IniFormat);
    int client = settings.value("ipc_client", 7704).toInt();

    m_socket = new QUdpSocket(this);
    m_socket->bind(QHostAddress::LocalHost, client);
    connect(m_socket, &QUdpSocket::readyRead, this, &QMQueue::queueEvent);
    connect(m_socket, &QUdpSocket::errorOccurred, this, &QMQueue::errorOccurred);
    qDebug() << __PRETTY_FUNCTION__ << ": BIND to" << m_socket->localPort();
}

QMQueue::~QMQueue()
{
}

void QMQueue::errorOccurred(QAbstractSocket::SocketError socketError)
{
    qWarning() << __PRETTY_FUNCTION__ << ": error" << socketError << ":" << m_socket->errorString();
}

void QMQueue::query()
{
    QString conf("/home/pi/.config/MythClock/MythClock.ini");
    QSettings settings(conf, QSettings::IniFormat);
    int server = settings.value("ipc_server", 7703).toInt();

    QByteArray ba("query", 5);
    QHostAddress ha(QHostAddress::LocalHost);
    QNetworkDatagram dg(ba, ha, server);
    m_socket->writeDatagram(dg);
}

void QMQueue::set(uint8_t bright)
{
    QString conf("/home/pi/.config/MythClock/MythClock.ini");
    QSettings settings(conf, QSettings::IniFormat);
    int server = settings.value("ipc_server", 7703).toInt();

    QByteArray ba;
    ba.setNum(bright);

    QHostAddress ha(QHostAddress::LocalHost);
    QNetworkDatagram dg(ba, ha, server);
    m_socket->writeDatagram(dg);
}

void QMQueue::queueEvent()
{
    bool ok;

    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        QByteArray ba = datagram.data();
        qDebug() << __PRETTY_FUNCTION__ << ": Got event" << ba;
        int v = ba.toInt(&ok);
        if (ok)
            emit value(v);
    }
}
