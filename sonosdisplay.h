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
    
    void go();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

protected slots:
    void requestFinished(QNetworkReply *reply);
    void albumArtFinished(QNetworkReply *reply);
    void requestSonosStatus();
    
signals:
    void startSonos();
    void endSonos();
    
private:
    void setupSonos();
    void calculateMinutes(int);
    void setURL(QString, QString room);
    void getAlbumArt(QUrl);
    void sonosRequestResult(QByteArray ba);
    
    QLabel *m_artist;
    QLabel *m_album;
    QLabel *m_station;
    QLabel *m_title;
    QLabel *m_albumArt;
    QLabel *m_elapsedTime;
    QGridLayout *m_layout;
    int m_duration;
    int m_elapsed;
    int m_trackNumber;
    int m_volume;
    QProgressBar *m_elapsedIndicator;
    
    QNetworkAccessManager *m_namMetaData;
    QNetworkAccessManager *m_namAlbumArt;
    QUrl m_url;
};

#endif // SONOSDISPLAY_H
