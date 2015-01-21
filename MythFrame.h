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

#ifndef MYTHFRAME_H_
#define MYTHFRAME_H_

#include <QtGui>
#include "MythClock.h"

class MythFrame : public QFrame {
public:
	MythFrame(QFrame *parent = 0);
	virtual ~MythFrame();

protected:
	void paintEvent(QPaintEvent*);
	void showEvent(QShowEvent*);

private:
	MythClock *clock;
};

#endif /* MYTHFRAME_H_ */
