TEMPLATE = app

CONFIG += gui debug core

QT += network widgets

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES = MythClock.cpp \
	MythFrame.cpp \
	LcdHandler.cpp \
	main.cpp
		
HEADERS = MythClock.h \
	MythFrame.h \
	LcdHandler.h \
