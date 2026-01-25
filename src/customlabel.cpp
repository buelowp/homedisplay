// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "customlabel.h"

CustomLabel::CustomLabel(const QString& text, int fontSize, int width, Qt::Alignment alignment, QWidget* parent) : QLabel(text, parent)
{
    QFont f("Roboto-Regular");
    if (width == 1280)
        fontSize = fontSize * 1.5;

    f.setPointSize(fontSize * 1.5);
    setFont(f);
    setAlignment(alignment);
    setScaledContents(true);
}

CustomLabel::CustomLabel(int fontSize, int width, Qt::Alignment alignment, QWidget* parent) : QLabel(parent)
{
    QFont f("Roboto-Regular");
    if (width == 1280)
        fontSize = fontSize * 1.5;

    f.setPointSize(fontSize * 1.5);
    setFont(f);
    setAlignment(alignment);
    setScaledContents(true);
}
