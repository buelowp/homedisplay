// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef SONOSDISPLAY_H
#define SONOSDISPLAY_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include "customlabel.h"

const QString g_progressBarStyle = "QProgressBar { border: 1px solid white;"
            "padding: 1px; border-radius: 5px; background: black; }"
            "QProgressBar::chunk {background: white;}";

class SonosDisplay : public QWidget
{
    Q_OBJECT
    
public:
    SonosDisplay(QWidget *parent = nullptr);
    ~SonosDisplay();

    void startTime();
    void stopTime();
    
public slots:
    void updateAlbum(QString a);
    void updateTitle(QString title);
    void updateArtist(QString artist);
    void updatePosition(QTime remain);
    void updateDuration(QTime duration);
    void updateAlbumArt(QPixmap &pixmap);
    void timeout();
    
private:
    void calculateMinutes(int);
    
    CustomLabel *m_artist;
    CustomLabel *m_album;
    CustomLabel *m_title;
    CustomLabel *m_time;
    QLabel *m_albumArt;
    CustomLabel *m_elapsedTime;
    QGridLayout *m_layout;
    QTime m_duration;
    QTime m_position;
    QProgressBar *m_elapsedIndicator;
    QTimer *m_timeTimer;
    int m_width;
};

#endif // SONOSDISPLAY_H
