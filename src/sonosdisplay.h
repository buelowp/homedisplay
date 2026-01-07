// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef SONOSDISPLAY_H
#define SONOSDISPLAY_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>

const QString g_progressBarStyle = "QProgressBar { border: 1px solid white;"
            "padding: 1px; border-radius: 5px; background: black; }"
            "QProgressBar::chunk {background: white;}";

class SonosDisplay : public QWidget
{
    Q_OBJECT
    
public:
    SonosDisplay(QWidget *parent = nullptr);
    ~SonosDisplay();
    
public slots:
    void updateAlbumArt(QString art);
    void updateAlbum(QString a);
    void updateTitle(QString title);
    void updateArtist(QString artist);
    void updatePosition(QTime remain);
    void updateDuration(QTime duration);
    
private:
    void calculateMinutes(int);
    
    QLabel *m_artist;
    QLabel *m_album;
    QLabel *m_title;
    QLabel *m_albumArt;
    QLabel *m_elapsedTime;
    QGridLayout *m_layout;
    QTime m_duration;
    QTime m_position;
    QProgressBar *m_elapsedIndicator;
};

#endif // SONOSDISPLAY_H
