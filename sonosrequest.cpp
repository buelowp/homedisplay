/*
 * Copyright (c) 2020 Peter Buelow <email>
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

#include "sonosrequest.h"

SonosRequest::SonosRequest(QObject *parent) : QObject(parent)
{
    m_running = false;
    m_manager = new QNetworkAccessManager(this);
    m_albumArt = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &SonosRequest::requestFinished);
    connect(m_albumArt, &QNetworkAccessManager::finished, this, &SonosRequest::albumArtFinished);
}

void SonosRequest::setURL(QString url, QString room)
{
    QString path = "/" + room + "/state";
    
    m_url = QUrl::fromUserInput(url + path);
    qDebug() << m_url;
}

void SonosRequest::run()
{
    QNetworkRequest request;
    request.setUrl(m_url);
    request.setRawHeader("User-Agent", "SonosRequest 1.0");
    m_manager->get(request);
    m_running = true;
}

void SonosRequest::getAlbumArt(QUrl url)
{
    qDebug() << __FUNCTION__ << ":" << url;
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "SonosRequest 1.0");
    m_albumArt->get(request);
}

void SonosRequest::albumArtFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->error());
        qDebug() << __FUNCTION__ << reply->error();
    }
    else {
        QByteArray ba = reply->readAll();
        emit (albumArt(ba));
    }
    reply->deleteLater();
}

void SonosRequest::requestFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->error());
        qDebug() << __FUNCTION__ << reply->error();
    }
    else {
        QByteArray ba = reply->readAll();
        emit (result(ba));
    }
    m_running = false;
    reply->deleteLater();
}

