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

ICONS = icons/5_1.jpg \
	icons/7_1.jpg \
	icons/ac3.png \
	icons/dts.png \
	icons/HD.png \
	icons/stereo.png

target.path = /usr/bin
INSTALLS += target

icons.path = /usr/share/mythclock
icons.files = icons/*
INSTALLS += icons
