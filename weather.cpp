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
                qDebug() << __PRETTY_FUNCTION__ << "Invalid json received";
            }
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        }
    }
}

void Weather::decodeResponse(QJsonDocument& doc)
{
    QJsonObject parent = doc.object();

    if (parent.contains("forecast")) {
        QJsonObject fc = parent["forecast"].toObject();
        if (fc.contains("forecastday")) {
            QJsonArray forecastday = fc["forecastday"].toArray();
            QJsonObject today = forecastday.at(0).toObject();
            if (today.contains("day")) {
                QJsonObject day = today["day"].toObject();
                double high = day["maxtemp_f"].toDouble();
                double low = day["mintemp_f"].toDouble();
                emit forecast(high, low);
                qDebug() << __PRETTY_FUNCTION__ << "Got a high:" << high << ", and a low:" << low;
            }
        }
    }
}

bool Weather::getToday()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");

    QString key = settings.value("apikey").toString();
    QString zip = settings.value("zip").toString();

    if (m_inProgress)
        return false;

    QUrl url(QString("https://api.weatherapi.com/v1/forecast.json?key=%1&q=%2&days=1&aqi=no&alerts=no").arg(key).arg(zip));
    qDebug() << __PRETTY_FUNCTION__ << url;
    if (url.isValid()) {
        m_manager->get(QNetworkRequest(url));
        return true;
    }
    return false;
}
