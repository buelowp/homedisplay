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

#include <QApplication>
#include "primarydisplay.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);
    PrimaryDisplay frame;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MythClock", "MythClock");

    qDebug() << __PRETTY_FUNCTION__ << ": debug is" << settings.value("debug").toBool();
    if (!settings.value("debug").toBool()) {
        frame.showFullScreen();
        app.setOverrideCursor(QCursor(Qt::BlankCursor));
    }
    else {
        frame.setFixedSize(800, 480);
        frame.show();
    }

    return app.exec();
}
