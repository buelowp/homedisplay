TEMPLATE = app

CONFIG += gui debug core

QT += network widgets

SOURCES = MythClock.cpp \
	MythFrame.cpp \
	LcdHandler.cpp \
	main.cpp
		
HEADERS = MythClock.h \
	MythFrame.h \
	LcdHandler.h
