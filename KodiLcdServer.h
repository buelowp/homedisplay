/*
 * Copyright (c) 2019 <copyright holder> <email>
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

#ifndef KODILCDSERVER_H
#define KODILCDSERVER_H

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>

#define ONE_SECOND  1000
#define ONE_MINUTE  (ONE_SECOND * 60)
#define PING_ID             1
#define PLAYER_ID           2
#define INT_METADATA_ID     3

/**
 * @todo write docs
 */
class KodiLcdServer : public QObject
{
    Q_OBJECT
public:
    KodiLcdServer(QObject *parent = 0);
    ~KodiLcdServer();
    
public slots:
    void kodiError(QAbstractSocket::SocketError socketError);
    void kodiConnected();
    void connectionClosed();
    void kodiResponse();
    void ping();
    void exec();
    void testForPlayback();
    void getPlaybackMetaData();
    
signals:
    void connectionError();
    void apiError();
    void progressPercentComplete(int);
    void metaDataEnded();
	void videoFormat(QString);
	void audioFormat(QString);
	void stereoFormat(QString);
	void playbackFlags(QString);
	void metaDataStarted();
	void progressTimeLeft(QByteArray);
	void progressTotalTime(QByteArray);
	void channelNumber(QByteArray);
	void showTitle(QByteArray);
	void showSubTitle(QByteArray);
    void recordedEvent();
    void clientConnected();
    
private:
    void playerData(QJsonArray&);
    void metaDataStartedEvent(QJsonObject);
    void requestAudioMetaData();
    void requestVideoMetaData();
    void requestPlaybackProperties();
    void parsePlaybackMetaData(QJsonObject);
    
    QTimer *m_connectionTimer;
    QTimer *m_pingTimer;
    QTimer *m_metadataTimer;
    QTimer *m_mediaCheck;
    QTcpSocket *m_kodi;
    
    qint64 m_lastPing;
    bool m_connected;
    bool m_mediaPlaybackStarted;
    int m_playerId;
};

#endif // KODILCDSERVER_H
