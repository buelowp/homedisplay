// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>

class ClockDisplay : public QFrame
{
    Q_OBJECT

public:
    ClockDisplay(QFrame *parent = nullptr);
    ~ClockDisplay();

    void updateDisplay(QString &topic, QJsonObject &object);

signals:
    void startDimScreen();

public slots:
    void clockTimeout();
    void showEvent(QShowEvent *e) override;

private:
    QGridLayout *m_layout;

	QLabel *m_primaryClock;
	QLabel *m_primaryDate;
	QLabel *m_lbCountdown;
    QLabel *m_rain;
    QLabel *m_temperature;
    QLabel *m_humidity;
    QLabel *m_uvIndex;
    QTimer *m_clockTimer;
    QDateTime m_time;
};

#endif // CLOCKDISPLAY_H
