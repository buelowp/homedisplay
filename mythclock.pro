TEMPLATE = app

CONFIG += gui debug core

QT += network widgets qmqtt

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES = AnalogClock.cpp \
	MythFrame.cpp \
	MythLcdServer.cpp \
	main.cpp \
	qmqttsubscriber.cpp \
	KodiLcdServer.cpp
		
HEADERS = AnalogClock.h \
	MythFrame.h \
	MythLcdServer.h \
	qmqttsubscriber.h \
	KodiLcdServer.h

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
