// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef WEATHER_H
#define WEATHER_H

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>

/**
 * @todo write docs
 */
class Weather : public QObject
{
    Q_OBJECT

public:
    Weather(QObject *parent = nullptr);
    ~Weather();

    bool getToday();

public slots:
    void requestFinished(QNetworkReply *reply);

signals:
    void forecast(double high, double low);
    void apiError();

private:
    void decodeResponse(QJsonDocument &doc);

    QNetworkAccessManager *m_manager;
    bool m_inProgress;
};

#endif // WEATHER_H
