/*
 * Copyright (c) 2020 Peter Buelow <email>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sonoslabel.h"

SonosLabel::SonosLabel(const QString &text, QWidget *parent, Qt::WindowFlags f) : 
    QLabel(text, parent, f)
{
    m_defaultPointSize = 12;
}

SonosLabel::SonosLabel(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent, f)
{
    m_defaultPointSize = 12;
}

void SonosLabel::resizeEvent(QResizeEvent *e)
{
    QFont f = font();
    QFontMetrics fm(font());
    QLabel::resizeEvent(e);
    if (fm.width(text()) > width()) {
        float factor = (float)width() / (float)fm.width(text());
        if (factor <= .6) {
            factor = .6;
        }
        f.setPointSizeF((m_defaultPointSize * factor) * .9);
    }
    else
        f.setPointSize(m_defaultPointSize);

    setFont(f);
}
