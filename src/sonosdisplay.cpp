// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "sonosdisplay.h"

SonosDisplay::SonosDisplay(QWidget *parent) : QWidget(parent)
{
    QFont c("Roboto-Regular", 24);
    QFont t("Roboto-Regular", 36);

    setObjectName("sonosdisplay");
    m_layout = new QGridLayout();
    m_layout->setObjectName("layout");
    m_title = new QLabel();
    m_title->setScaledContents(true);
    m_title->setObjectName("title");
    m_artist = new QLabel();
    m_artist->setScaledContents(true);
    m_artist->setObjectName("artist");
    m_album = new QLabel();
    m_album->setScaledContents(true);
    m_album->setObjectName("album");
    m_elapsedTime = new QLabel();
    m_elapsedTime->setScaledContents(true);
    m_elapsedTime->setObjectName("et");
    m_albumArt = new QLabel();
    m_albumArt->setFixedSize(200, 200);
    m_albumArt->setObjectName("albumart");
    m_elapsedIndicator = new QProgressBar();
    m_elapsedIndicator->setStyleSheet(g_progressBarStyle);
    m_elapsedIndicator->setTextVisible(false);
    m_elapsedIndicator->setObjectName("ei");
    
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
}

SonosDisplay::~SonosDisplay()
{
}

void SonosDisplay::updateAlbumArt(QString art)
{
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
