// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>

#include "inside.h"

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
    void temperature(float temp);
    void humidity(float humidity);

private:
    Inside *m_sht31d;
    QGridLayout *m_layout;

	QLabel *m_primaryClock;
	QLabel *m_primaryDate;
	QLabel *m_lbCountdown;
    QLabel *m_insideHumidity;
    QLabel *m_temperature;
    QLabel *m_humidity;
    QLabel *m_insideTemp;
    QTimer *m_clockTimer;
    QLabel *m_inside;
    QLabel *m_outside;
    QDateTime m_time;
};

#endif // CLOCKDISPLAY_H
