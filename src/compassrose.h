// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef COMPASSROSE_H
#define COMPASSROSE_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

class CompassRose : public QWidget
{
    Q_OBJECT

public:
    CompassRose(QWidget *parent = nullptr);
    ~CompassRose();
    
    void setAngle(int angle);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    
private:
    int m_angle;
};

#endif // COMPASSROSE_H
