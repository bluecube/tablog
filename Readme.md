# Tablog

A C++17 library for writing and reading compressed tables of (prefereably
continuous) numerical values.

For example logging streams of sensor data on microcontrollers.

Goals:
1. Fast compression with small memory footprint
2. Decent compression ratio
3. Streamable (decoding can be picked up from the midlle of the stream, after reading limited amount of data)

Intended for ESP32, but probably will work all right elsewhere too.
