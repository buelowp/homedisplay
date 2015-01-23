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

void printFonts()
{
     QFontDatabase database;

     foreach (const QString &family, database.families()) {
    	 qDebug() << "Family:" << family;

         foreach (const QString &style, database.styles(family)) {
        	 qDebug() << " Style:" << style;
         }
     }
}

int main(int argc, char **argv)
{
    QApplication app (argc, argv);
    MythFrame frame;

 //   printFonts();

    frame.setGeometry(0, 0, 480, 272);
    if (!frame.init())
    	return -1;

    frame.show();

    return app.exec();
}
