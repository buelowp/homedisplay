// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "sonosdisplay.h"

SonosDisplay::SonosDisplay(QWidget *parent) : QWidget(parent)
{
    QFont c("Roboto-Regular", 24);
    QFont t("Roboto-Regular", 36);

    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    if (primaryScreen) {
        // Get the screen's full geometry (resolution in pixels)
        QRect screenGeometry = primaryScreen->geometry();
        m_width = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        qDebug() << __PRETTY_FUNCTION__ << "Screen Resolution:" << m_width << "x" << screenHeight;
    }

    setWindowState(Qt::WindowFullScreen);
    setObjectName("sonosdisplay");
    m_layout = new QGridLayout();
    m_title = new CustomLabel(36, m_width, Qt::AlignLeft);
    m_artist = new CustomLabel(24, m_width, Qt::AlignLeft);
    m_album = new CustomLabel(24, m_width, Qt::AlignLeft);
    m_elapsedTime = new CustomLabel(24, m_width, Qt::AlignLeft);
    m_time = new CustomLabel(24, m_width, Qt::AlignRight);
    m_albumArt = new QLabel();
    m_albumArt->setFixedSize(300, 300);
    m_albumArt->setAlignment(Qt::AlignCenter);
    m_elapsedIndicator = new QProgressBar();
    m_elapsedIndicator->setStyleSheet(g_progressBarStyle);
    m_elapsedIndicator->setTextVisible(false);
    
    m_layout->setSpacing(20);
    m_layout->addWidget(m_title, 0, 0, 1, 4);
    m_layout->addWidget(m_albumArt, 1, 0, 3, 1);
    m_layout->addWidget(m_artist, 1, 1, 1, 3);
    m_layout->addWidget(m_album, 2, 1, 1, 3);
    m_layout->addWidget(m_elapsedTime, 3, 1, 1, 3);
    m_layout->addWidget(m_time, 4, 1, 1, 3);
    m_layout->addWidget(m_elapsedIndicator, 4, 0, 1, 4);
    m_layout->setAlignment(m_albumArt, Qt::AlignTop);
    setLayout(m_layout);

    m_timeTimer = new QTimer();
    m_timeTimer->setInterval(1000);
    connect(m_timeTimer, &QTimer::timeout, this, &SonosDisplay::timeout);
}

SonosDisplay::~SonosDisplay()
{
}

void SonosDisplay::startTime()
{
    timeout();
    m_timeTimer->start();
}

void SonosDisplay::stopTime()
{
    m_timeTimer->stop();
}

void SonosDisplay::timeout()
{
    QTime now = QTime::currentTime();

    m_time->setText(now.toString("h:mm:ss"));
}

void SonosDisplay::updateAlbumArt(QPixmap &pixmap)
{
    m_albumArt->setPixmap(pixmap.scaledToWidth(300));
}

void SonosDisplay::updateTitle(QString title)
{
    m_title->setText(title);
}

void SonosDisplay::updateArtist(QString artist)
{
    m_artist->setText(artist);
}

void SonosDisplay::updateAlbum(QString album)
{
    m_album->setText(album);
}

void SonosDisplay::updatePosition(QTime elapsed)
{
    if (elapsed.isValid()) {
        m_position = elapsed;
        m_elapsedTime->setText(elapsed.toString("h:mm:ss"));
        m_elapsedIndicator->setValue(elapsed.msecsSinceStartOfDay() / 1000);
    }
}

void SonosDisplay::updateDuration(QTime duration)
{
    if (duration != m_duration) {
        m_duration = duration;
        m_elapsedIndicator->setRange(0, duration.msecsSinceStartOfDay() / 1000);
        m_elapsedIndicator->setValue(0);
    }
}
