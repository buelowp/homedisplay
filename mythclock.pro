TEMPLATE = app

CONFIG += gui debug core

QT += network

SOURCES = MythClock.cpp \
	MythFrame.cpp \
	LcdHandler.cpp \
	main.cpp
		
HEADERS = MythClock.h \
	MythFrame.h \
	LcdHandler.h
