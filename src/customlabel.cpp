// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "customlabel.h"

CustomLabel::CustomLabel(const QString& text, int fontSize, Qt::Alignment alignment, QWidget* parent) : QLabel(text, parent)
{
    QFont f("Roboto-Regular");
    f.setPointSize(fontSize);
    setFont(f);
    setAlignment(alignment);
}

CustomLabel::CustomLabel(int fontSize, Qt::Alignment alignment, QWidget* parent) : QLabel(parent)
{
    QFont f("Roboto-Regular");
    f.setPointSize(fontSize);
    setFont(f);
    setAlignment(alignment);
}
