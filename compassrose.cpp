// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "compassrose.h"

CompassRose::CompassRose(QWidget *parent) : QWidget(parent)
{
    m_angle = 0;
}

CompassRose::~CompassRose()
{
}

QSize CompassRose::sizeHint() const
{
    return QSize(200,200);
}

void CompassRose::setAngle(int angle)
{
    m_angle = angle;
    repaint();
}

void CompassRose::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    static const QPoint windDir[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -40)
    };

    QColor roseColor(127, 0, 127);
    QColor hashColor(50, 50, 50);
    int side = qMin(width(), height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setPen(roseColor);
    painter.setBrush(roseColor);

    painter.save();
    painter.rotate(m_angle);
    painter.drawConvexPolygon(windDir, 3);
    painter.restore();
    
    painter.setBrush(Qt::white);
    painter.setPen(Qt::white);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }
}
