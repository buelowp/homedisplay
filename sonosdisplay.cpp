// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "sonosdisplay.h"

SonosDisplay::SonosDisplay(QWidget *parent) : QWidget(parent)
{
    QFont c("Roboto-Regular", 24);
    QFont t("Roboto-Regular", 36);

    m_namAlbumArt = new QNetworkAccessManager(this);
    m_namMetaData = new QNetworkAccessManager(this);
    connect(m_namMetaData, &QNetworkAccessManager::finished, this, &SonosDisplay::requestFinished);
    connect(m_namAlbumArt, &QNetworkAccessManager::finished, this, &SonosDisplay::albumArtFinished);
    
    m_layout = new QGridLayout();
    m_title = new QLabel();
    m_title->setScaledContents(true);
    m_artist = new QLabel();
    m_artist->setScaledContents(true);
    m_album = new QLabel();
    m_album->setScaledContents(true);
    m_elapsedTime = new QLabel();
    m_elapsedTime->setScaledContents(true);
    m_albumArt = new QLabel();
    m_albumArt->setFixedSize(200, 200);
    m_elapsedIndicator = new QProgressBar();
    m_elapsedIndicator->setStyleSheet(g_progressBarStyle);
    m_elapsedIndicator->setTextVisible(false);
    
    m_artist->setFont(c);
    m_album->setFont(c);
    m_title->setFont(t);
    m_elapsedTime->setFont(c);
    
    m_layout->setSpacing(20);
    m_layout->addWidget(m_title, 0, 0, 1, 4);
    m_layout->addWidget(m_albumArt, 1, 0, 3, 1);
    m_layout->addWidget(m_artist, 1, 1, 1, 3);
    m_layout->addWidget(m_album, 2, 1, 1, 3);
    m_layout->addWidget(m_elapsedTime, 3, 1, 1, 3);
    m_layout->addWidget(m_elapsedIndicator, 4, 0, 1, 4);
    m_layout->setAlignment(m_albumArt, Qt::AlignTop);
    setLayout(m_layout);

    setupSonos();
}

SonosDisplay::~SonosDisplay()
{
}

void SonosDisplay::go()
{
    requestSonosStatus();
}

void SonosDisplay::requestSonosStatus()
{
    QNetworkRequest request;
    request.setUrl(m_url);
    request.setRawHeader("User-Agent", "SonosRequest 1.0");
    m_namMetaData->get(request);
}

void SonosDisplay::setURL(QString url, QString room)
{
    QString path = "/" + room + "/state";
    
    m_url = QUrl::fromUserInput(url + path);
    qDebug() << __PRETTY_FUNCTION__ << ":" << m_url;
}

void SonosDisplay::sonosRequestResult(QByteArray ba)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    QJsonParseError *error = new QJsonParseError();
    static QString lastState = "none";
    static QString lastTitle;

    QJsonDocument doc = QJsonDocument::fromJson(ba, error);
    if (error->error != QJsonParseError::NoError) {
        qDebug() << __PRETTY_FUNCTION__ << ":" << error->errorString();
    }
    else {
        if (doc.isObject()) {
            QJsonObject parent = doc.object();
            QString playback = parent["playbackState"].toString();
            QJsonObject current = parent["currentTrack"].toObject();
            if (playback == "PLAYING") {
                if (current["type"].toString() == "radio") {
                    if (lastTitle != current["title"].toString()) {
                        m_artist->setText(current["artist"].toString());
                        m_album->setText(current["album"].toString());
      	                m_title->setText(current["title"].toString());
                        QUrl url(settings.value("sonosaddress").toString() + current["albumArtUri"].toString());
                        getAlbumArt(url);
                        m_elapsedIndicator->setVisible(false);
                        m_elapsedTime->setText(current["elapsedTimeFormatted"].toString());
                        lastTitle = current["title"].toString();
                    }
                }
                else {
                    if (parent["trackNo"] != m_trackNumber) {
                         m_artist->setText(current["artist"].toString());
                         m_album->setText(current["album"].toString());
                         m_title->setText(current["title"].toString());
                         m_duration = current["duration"].toInt();
                         m_elapsedIndicator->setMaximum(m_duration);
                         m_elapsedIndicator->setMinimum(0);
                         m_trackNumber = parent["trackNo"].toInt();
                         QUrl url(settings.value("sonosaddress").toString() + current["albumArtUri"].toString());
                         getAlbumArt(url);
                    }
                    m_elapsedIndicator->setVisible(true);
                    m_elapsedTime->setVisible(true);
                    calculateMinutes(parent["elapsedTime"].toInt());
                    m_volume = parent["volume"].toInt();
                }
                if (lastState != playback) {
                    lastState = playback;
                    emit startSonos();
                }
            }
            else {
                if (lastState != playback) {
                    emit endSonos();
                    lastState = playback;
                }
            }
        }
    }
}

void SonosDisplay::setupSonos()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");
    QString hostname = settings.value("sonosserver").toString();
    QString port = settings.value("sonosport").toString();
    QHostInfo lookup = QHostInfo::fromName(hostname);
    QList<QHostAddress> addresses = lookup.addresses();
    
    if (addresses.size() > 0) {
        QString url = QString("%1:%2").arg(addresses.at(0).toString()).arg(settings.value("sonosport").toInt());
        setURL(url, settings.value("sonosroom").toString());
        qDebug() << __PRETTY_FUNCTION__ << ": setting host address to" << addresses.at(0);
    }
    else {
        setURL(QString("%1:%2").arg(hostname).arg(port), settings.value("sonosroom").toString());
        qDebug() << __PRETTY_FUNCTION__ << ": using" << hostname << ":" << port;
    }
}

void SonosDisplay::calculateMinutes(int elapsed)
{
    QString display = QString("%1:%2").arg(elapsed/60, 2, 10, QChar('0')).arg(elapsed%60, 2, 10, QChar('0'));
//    m_elapsedIndicator->setFormat(display);
    m_elapsedTime->setText(display);
    m_elapsedIndicator->setValue(elapsed);
}

void SonosDisplay::getAlbumArt(QUrl url)
{
    QByteArray base64;

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "SonosRequest 1.0");
    m_namAlbumArt->get(request);
}

void SonosDisplay::albumArtFinished(QNetworkReply* reply)
{
    QDateTime now = QDateTime::currentDateTime();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << __PRETTY_FUNCTION__ << ":" << now << ":" << reply->error();
    }
    else {
        QByteArray ba = reply->readAll();
        QPixmap art;
    
        art.loadFromData(ba);
        m_albumArt->setPixmap(art.scaledToWidth(200));;
    }
    reply->deleteLater();
}

void SonosDisplay::requestFinished(QNetworkReply* reply)
{
    QDateTime now = QDateTime::currentDateTime();
    
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << __PRETTY_FUNCTION__ << ":" << reply->error();
    	QTimer::singleShot(1000, this, &SonosDisplay::requestSonosStatus);
    }
    else {
        QByteArray ba = reply->readAll();
        sonosRequestResult(ba);
    	QTimer::singleShot(100, this, &SonosDisplay::requestSonosStatus);
    }

    reply->deleteLater();
}
