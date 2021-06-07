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
#include "MythFrame.h"
#include "sonosrequest.h"

int main(int argc, char **argv)
{
    qputenv("QT_LOGGING_TO_CONSOLE", QByteArray("2"));
    // It takes the network a little while to get settled
    // so lets give it some time to figure life out

    QApplication app (argc, argv);
    QThread::sleep(30);
    MythFrame frame;

    app.setOverrideCursor(QCursor(Qt::BlankCursor));
    frame.setGeometry(0, 0, 1024, 600);
    frame.setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    frame.show();

    return app.exec();
}
