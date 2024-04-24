// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef BIGCLOCK_H
#define BIGCLOCK_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

/**
 * @todo write docs
 */
class BigClock : public QFrame
{
    Q_OBJECT

public:
    BigClock(QFrame *parent = nullptr);
    ~BigClock();

public slots:
    void updateClock();

protected:
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

private:
    QGridLayout *m_layout;

	QLabel *m_primaryClock;
	QLabel *m_primaryDate;
    QTimer *m_updateTimer;
    QDateTime m_time;
};

#endif // BIGCLOCK_H
