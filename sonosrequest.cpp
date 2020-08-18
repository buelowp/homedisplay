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

    m_pendingArtworkCacheFile = new QFile();
}

void SonosRequest::setURL(QString url, QString room)
{
    QString path = "/" + room + "/state";
    
    m_url = QUrl::fromUserInput(url + path);
    qDebug() << __PRETTY_FUNCTION__ << ":" << m_url;
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
    QByteArray base64 = url.query().toUtf8().toBase64(QByteArray::Base64Encoding|QByteArray::OmitTrailingEquals);
    QString encoded(base64);
    m_pendingArtworkCacheFile->setFileName(g_cachePath + "/" + encoded);
    if (m_pendingArtworkCacheFile->exists()) {
        if (m_pendingArtworkCacheFile->open(QIODevice::ReadOnly)) {
            qDebug() << __PRETTY_FUNCTION__ << ":" << "Reading albumart from" << m_pendingArtworkCacheFile->fileName();
            QByteArray ba = m_pendingArtworkCacheFile->readAll();
            emit albumArt(ba);
            m_pendingArtworkCacheFile->close();
        }
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ":" << "Requesting albumart from the network";
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("User-Agent", "SonosRequest 1.0");
        m_albumArt->get(request);
    }
}

void SonosRequest::albumArtFinished(QNetworkReply* reply)
{
    QDateTime now = QDateTime::currentDateTime();

    if (reply->error() != QNetworkReply::NoError) {
        emit albumArtError(reply->error());
        qDebug() << __PRETTY_FUNCTION__ << ":" << now << ":" << reply->error();
    }
    else {
        QByteArray ba = reply->readAll();
        storeAlbumArt(reply->url(), ba);
        emit (albumArt(ba));
    }
    reply->deleteLater();
}

void SonosRequest::requestFinished(QNetworkReply* reply)
{
    QDateTime now = QDateTime::currentDateTime();
    
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->error());
        qDebug() << __PRETTY_FUNCTION__ << ":" << now << ":" << reply->error();
    }
    else {
        QByteArray ba = reply->readAll();
        emit (result(ba));
    }
    m_running = false;
    reply->deleteLater();
}

void SonosRequest::storeAlbumArt(QUrl url, QByteArray &content)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << "Storing art to cache file" << m_pendingArtworkCacheFile->fileName();
    if (m_pendingArtworkCacheFile->open(QIODevice::WriteOnly)) {
        m_pendingArtworkCacheFile->write(content);
        m_pendingArtworkCacheFile->close();
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ":" << "Could not open" << m_pendingArtworkCacheFile->fileName();
    }
}