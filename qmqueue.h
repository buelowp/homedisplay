// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef QMQUEUE_H
#define QMQUEUE_H

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>

class QMQueue : public QObject
{
    Q_OBJECT

public:
    QMQueue(QObject *parent = nullptr);
    ~QMQueue();

    void query();

public slots:
    void set(uint8_t bright);
    void queueEvent();
    void errorOccurred(QAbstractSocket::SocketError socketError);


signals:
    void value(uint8_t bright);

private:
    QUdpSocket *m_socket;
    uint8_t m_brightness;
    bool m_open;
};

#endif // QMQUEUE_H
