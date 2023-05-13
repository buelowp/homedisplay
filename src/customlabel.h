// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>


class CustomLabel : public QLabel
{
    Q_OBJECT

public:
    CustomLabel(const QString &text, int fontSize, Qt::Alignment alignment = Qt::AlignCenter, QWidget *parent = nullptr);
    CustomLabel(int fontSize, Qt::Alignment alignment = Qt::AlignCenter, QWidget *parent = nullptr);
};

#endif // CUSTOMLABEL_H
