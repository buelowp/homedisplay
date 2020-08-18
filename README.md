# Raspberry PI Metadata Clock
Raspberry PI clock that can also show you local weather and handle sonos metadata if you would like

Local Weather is handled by a little Particle micro which is checking temps in my backyard and sending them out
over MQTT for me. This won't work unless you have an MQTT service along with something which tells you what
the temperature is

Font resizing is generously borrowed from https://github.com/jonaias/DynamicFontSizeWidgets. I didn't use
the entire implementation, as I don't need it. I borrowed the idea which fixed my use of resizeEvent
which may get stuck in an infinite loop.

Uses https://www.hackster.io/mark-hank/sonos-album-art-on-raspberry-pi-screen-5b0012 for handling sonos metadata
