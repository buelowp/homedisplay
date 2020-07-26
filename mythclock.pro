TEMPLATE = app

CONFIG += gui debug core

QT += network widgets qmqtt

OBJECTS_DIR = .obj
MOC_DIR = .moc

LIBS = -lqutesonos

SOURCES = MythFrame.cpp \
	main.cpp \
	qmqttsubscriber.cpp
		
HEADERS = MythFrame.h \
	qmqttsubscriber.h

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
