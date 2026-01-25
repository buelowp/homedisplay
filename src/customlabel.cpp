// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "customlabel.h"

CustomLabel::CustomLabel(const QString& text, int fontSize, int width, Qt::Alignment alignment, QWidget* parent) : QLabel(text, parent)
{
    QFont f("Roboto-Regular");
    double multiplyer = static_cast<double>(width) / 800.0;
    double newsize = static_cast<double>(fontSize) * multiplyer;

    f.setPointSize(static_cast<int>(newsize));
    setFont(f);
    setAlignment(alignment);
    setScaledContents(true);
}

CustomLabel::CustomLabel(int fontSize, int width, Qt::Alignment alignment, QWidget* parent) : QLabel(parent)
{
    QFont f("Roboto-Regular");
    double multiplyer = static_cast<double>(width) / 800.0;
    double newsize = static_cast<double>(fontSize) * multiplyer;

    f.setPointSize(static_cast<int>(newsize));
    setFont(f);
    setAlignment(alignment);
    setScaledContents(true);
}
