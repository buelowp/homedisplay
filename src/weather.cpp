// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "weather.h"

Weather::Weather(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &Weather::requestFinished);
    m_inProgress = false;
}

Weather::~Weather()
{
}

void Weather::requestFinished(QNetworkReply* reply)
{
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );

    if (statusCode.isValid()) {
        if (statusCode.toInt() == 200) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject()) {
                decodeResponse(doc);
            }
            else {
                qDebug() << "Invalid json received";
            }
        }
        else {
            qDebug() << reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        }
    }
}

void Weather::decodeResponse(QJsonDocument& doc)
{
    QJsonObject parent = doc.object();
    double high = 0.0;
    double low = 0.0;

    if (parent.contains("daily")) {
        QJsonObject daily = parent["daily"].toObject();
        if (daily.contains("temperature_2m_max")) {
            if (daily["temperature_2m_max"].isArray()) {
                QJsonArray max = daily["temperature_2m_max"].toArray();
                high = max[0].toDouble();
            }
        }
        if (daily["temperature_2m_min"].isArray()) {
            if (daily["temperature_2m_min"].isArray()) {
                QJsonArray min = daily["temperature_2m_min"].toArray();
                low = min[0].toDouble();
            }
        }
        if (daily.contains("precipitation_probability_max")) {
            if (daily["precipitation_probability_max"].isArray()) {
                QJsonArray p = daily["precipitation_probability_max"].toArray();
                emit precip(p[0].toDouble());
            }
        }

        emit (forecast(high, low));
    }
}

bool Weather::getToday()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");

    QString key = settings.value("apikey").toString();
    QString zip = settings.value("zip").toString();
    double lat = settings.value("lat").toDouble();
    double lon = settings.value("lon").toDouble();
    QDateTime now = QDateTime::currentDateTime();
    QTimeZone zone = now.timeZone();
    QString tz = zone.id();
    tz.replace(QString("/"), QString("%2f"));

    if (m_inProgress)
        return false;

    QUrl url(QString("https://api.open-meteo.com/v1/forecast?latitude=%1&longitude=%2&daily=temperature_2m_max,temperature_2m_min&current_weather=true&temperature_unit=fahrenheit&forecast_days=1&timezone=%3").arg(lat, 0, 'f', 4).arg(lon, 0, 'f', 4).arg(QString(tz)));
    if (url.isValid()) {
        m_manager->get(QNetworkRequest(url));
        return true;
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << "url has issues";
    }
    return false;
}
