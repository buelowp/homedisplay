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
#include "sonosrequest.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);
    PrimaryDisplay frame;

    QDir cache(g_cachePath);
    if (!cache.exists()) {
        cache.mkpath(g_cachePath);
    }
//    app.setOverrideCursor(QCursor(Qt::BlankCursor));
    frame.setFixedSize(800, 480);
    frame.show();
//    frame.showFullScreen();

    return app.exec();
}
