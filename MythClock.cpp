/*
    This file is part of MythClock.

    MythClock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MythClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MythClock.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "MythClock.h"

MythClock::MythClock(QWidget *parent) : QWidget(parent) {
	QPalette pal(palette());
	pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

	pTimer = new QTimer(this);
	connect(pTimer, SIGNAL(timeout()), this, SLOT(update()));
	pTimer->setInterval(1000);
	pTimer->start();
	resize(480, 272);
}

MythClock::~MythClock() {
}

void MythClock::paintEvent(QPaintEvent *event)
{
    static const QPoint hourHandFront[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -((height() / 4) - 2))
    };
    static const QPoint minuteHandFront[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -((height() / 3) - 2))
    };
    static const QPoint hourHandBack[3] = {
        QPoint(8, 9),
        QPoint(-8, 9),
        QPoint(0, -((height() / 4) + 1))
    };
    static const QPoint minuteHandBack[3] = {
        QPoint(8, 9),
        QPoint(-8, 9),
        QPoint(0, -((height() / 3) + 1))
    };

    QColor hourColorFront(Qt::white);
    QColor minuteColorFront(Qt::white);
    QColor hourColorBack(Qt::gray);
    QColor minuteColorBack(Qt::gray);

    int side = qMin(width(), height());
    QTime time = QTime::currentTime();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setPen(hourColorFront);

    for (int i = 0; i < 12; ++i) {
    	painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColorBack);
    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawConvexPolygon(minuteHandBack, 3);
    painter.restore();

    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColorFront);
    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawConvexPolygon(minuteHandFront, 3);
    painter.restore();

    painter.setPen(minuteColorFront);

    for (int j = 0; j < 60; ++j) {
    	if ((j % 5) != 0)
    		painter.drawLine(92, 0, 96, 0);

    	painter.rotate(6.0);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColorBack);
    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawConvexPolygon(hourHandBack, 3);
    painter.restore();

    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColorFront);
    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawConvexPolygon(hourHandFront, 3);
    painter.restore();

    QRect button(QPoint(-15, -15), QSize(30, 30));
    painter.setPen(Qt::black);
    painter.setBrush(QColor(Qt::black));
    painter.save();
    painter.drawEllipse(button);
    painter.restore();
}
