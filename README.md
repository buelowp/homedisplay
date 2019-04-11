# Raspberry PI Metadata Clock
What do you do if you run Kodi or Myth and want to know the metadata in a new window for the currently playing video? Well, run this and you can connect to Myth or Kodi and request the data to show when you play a movie. While no movie is playing, it just shows an accurate clock.

It can listen for MythTV connections, which requires a few database changes to make work. It can try to connect to kodi directly, and if not there, just sits idly by as a clock.

## MythTV

You must go into your MythTV database and add the following two rows

TBD

Once done, MythTV will attempt to connect to your PI and show you metadata.

## Kodi

Add kodiserver and kodiport to your MythClock ini file and it will attempt to connect to Kodi directly.

## Normal Operation

Both will work in the background and listen for media playback. Right now, it's not terribly good at figuring out what's going on, it just watches for movies or TV. However, it can be smarter and eventually I will try to do that.

## Dependencies

This project depends on qmqtt to provide an MQTT connection. If you add the MQTT server to the MythClock.ini file, you will try to connect to MQTT to listen for lightning events. It's kinda cool.
