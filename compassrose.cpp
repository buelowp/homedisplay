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

    int side = qMin(width(), height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setPen(Qt::NoPen);
    painter.setPen(roseColor);
    painter.setBrush(roseColor);

    painter.save();
    painter.rotate(30.0 * (m_angle / 60.0));
    painter.drawConvexPolygon(windDir, 3);
    painter.restore();
}
