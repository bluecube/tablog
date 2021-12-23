# Tablog

A C++17 library for writing and reading compressed tables of (prefereably
continuous) numerical values, for example logging streams of sensor data on microcontrollers.

Goals:
1. Fast compression with small memory footprint
2. Decent compression ratio (hopefully better than GZipped CSV)
3. Streamable (decoding can be picked up from the midlle of the stream, after reading limited amount of data)

Should work well on ESP32, ARM (Raspberry PI), x64 (mostly for development and testing) and (with lower priority) AVR.
