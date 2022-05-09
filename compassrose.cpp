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

    QColor roseColor(255, 255, 127);
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
    
    painter.setFont(QFont("Roboto-Regular", 8));
    painter.setBrush(Qt::white);
    painter.setPen(Qt::white);
    double radius = std::min(width(), height()) / 3;
    for (int i = 0; i < 12; ++i) {
        int numeral = i + 1;
        double radians = numeral * 2.0 * 3.141592654 / 12;

        /*
         * Calculate the position of the text centre as it would be required
         * in the absence of a transform.
         */
        QPoint pos = rect().center() + QPoint(radius * std::sin(radians), -radius * std::cos(radians));

        /*
         * Set up the transform.
         */
        QTransform t;
        t.translate(pos.x(), pos.y());
        t.rotateRadians(radians);
        painter.setTransform(t);

        /*
         * Specify a huge bounding rectangle centred at the origin.  The
         * transform should take care of position and orientation.
         */
        switch (numeral) {
            case 3:
                painter.drawText(QRect(-(INT_MAX / 2), -(INT_MAX / 2), INT_MAX, INT_MAX), Qt::AlignCenter, QString("E"));
                break;
            case 6:
                painter.drawText(QRect(-(INT_MAX / 2), -(INT_MAX / 2), INT_MAX, INT_MAX), Qt::AlignCenter, QString("S"));
                break;
            case 9:
                painter.drawText(QRect(-(INT_MAX / 2), -(INT_MAX / 2), INT_MAX, INT_MAX), Qt::AlignCenter, QString("W"));
                break;
            case 12:
                painter.drawText(QRect(-(INT_MAX / 2), -(INT_MAX / 2), INT_MAX, INT_MAX), Qt::AlignCenter, QString("N"));
                break;
        }
    }
/*
    painter.setFont(QFont("Roboto-Regular", 8));
    painter.setBrush(Qt::white);
    painter.setPen(Qt::white);
    for (int i = 0; i < 12; ++i) {
        if (i == 9)
            painter.drawText(88, 0, "N");
        else
            painter.drawLine(88, 0, 96, 0);
        
        painter.rotate(30.0);
    }
*/
}
