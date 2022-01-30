# Tablog

[![CI status](https://github.com/bluecube/tablog/actions/workflows/CI.yml/badge.svg?branch=master)](https://github.com/bluecube/tablog/actions/workflows/CI.yml)
[![Coverage Status](https://coveralls.io/repos/github/bluecube/tablog/badge.svg?branch=master)](https://coveralls.io/github/bluecube/tablog?branch=master)

A C++17 library for writing and reading compressed tables of (prefereably
continuous) numerical values, for example logging streams of sensor data on microcontrollers.

## Goals

1. Fast compression with small memory footprint
2. Decent compression ratio (better than GZipped CSV)
3. Streamable (decoding can be picked up from the midlle of the stream, after reading limited amount of data)
4. Should work well on ESP32, ARM (Raspberry PI), x64 (mostly for development and testing) and (with lower priority) AVR.

## Compression ratio overview

|Dataset|Tablog: Compressed size|gzip -9: Compressed size|
|-------|-----------------------|---------------------|
|`dataset1/drift.csv`|14795 B (-67.6 %)|45646 B|
|`dataset1/power.csv`|448931 B (-16.8 %)|539491 B|
|`dataset1/soc.csv`|19211 B (-29.9 %)|27394 B|
|`dataset1/tph.csv`|21925 B (-46.4 %)|40877 B|
|`gcode/parts.csv`|202272 B (-48.0 %)|388618 B|
|`phone_imu/imu.csv`|153224 B (-40.5 %)|257485 B|
|`phone_imu/magnetometer.csv`|49959 B (-49.8 %)|99575 B|
|`twinsen/18e755d7ecc2b2a3_data.csv`|320743 B (-73.7 %)|1218594 B|
|`twinsen/66d8276d3a2480c2_data.csv`|504106 B (-64.2 %)|1407575 B|
|`empty("u8")`|5 B (-75.0 %)|20 B|
|`empty("s16")`|5 B (-75.0 %)|20 B|
|`empty("u64")`|5 B (-75.0 %)|20 B|
|`empty("s64")`|5 B (-75.0 %)|20 B|
|`count_up("u8", length=100)`|18 B (-85.0 %)|120 B|
|`count_up("u8", length=5000)`|674 B (**+101.2 %**)|335 B|
|`count_up("s16", length=100)`|23 B (-85.4 %)|157 B|
|`count_up("s16", length=5000)`|635 B (-92.8 %)|8790 B|
|`count_up("u64", length=100)`|20 B (-89.3 %)|187 B|
|`count_up("u64", length=5000)`|633 B (-93.9 %)|10353 B|
|`count_up("s64", length=100)`|36 B (-80.9 %)|188 B|
|`count_up("s64", length=5000)`|649 B (-93.7 %)|10328 B|
|`minor7chord("u8", 100, length=100)`|37 B (-68.9 %)|119 B|
|`minor7chord("u8", 10000, length=100)`|22 B (-51.1 %)|45 B|
|`minor7chord("u8", 100, length=5000)`|1307 B (**+27.3 %**)|1027 B|
|`minor7chord("u8", 10000, length=5000)`|710 B (-3.0 %)|732 B|
|`minor7chord("s16", 100, length=100)`|141 B (-36.8 %)|223 B|
|`minor7chord("s16", 10000, length=100)`|34 B (-84.8 %)|223 B|
|`minor7chord("s16", 100, length=5000)`|6258 B (**+187.6 %**)|2176 B|
|`minor7chord("s16", 10000, length=5000)`|1263 B (-87.2 %)|9833 B|
|`minor7chord("u64", 100, length=100)`|1139 B (**+40.3 %**)|812 B|
|`minor7chord("u64", 10000, length=100)`|747 B (-8.3 %)|815 B|
|`minor7chord("u64", 100, length=5000)`|36759 B (**+73.8 %**)|21154 B|
|`minor7chord("u64", 10000, length=5000)`|28997 B (-21.3 %)|36832 B|
|`minor7chord("s64", 100, length=100)`|1126 B (**+36.8 %**)|823 B|
|`minor7chord("s64", 10000, length=100)`|730 B (-11.3 %)|823 B|
|`minor7chord("s64", 100, length=5000)`|36748 B (**+67.3 %**)|21966 B|
|`minor7chord("s64", 10000, length=5000)`|28980 B (-22.6 %)|37426 B|
|`random("u8", length=100)`|138 B (**+12.2 %**)|123 B|
|`random("u8", length=5000)`|6406 B (**+27.5 %**)|5023 B|
|`random("s16", length=100)`|260 B (**+16.6 %**)|223 B|
|`random("s16", length=5000)`|11455 B (**+14.3 %**)|10023 B|
|`random("u64", length=100)`|1336 B (**+62.3 %**)|823 B|
|`random("u64", length=5000)`|42130 B (+5.2 %)|40033 B|
|`random("s64", length=100)`|1327 B (**+61.2 %**)|823 B|
|`random("s64", length=5000)`|42101 B (+5.2 %)|40033 B|
|`random_smooth("u8", 100, length=100)`|30 B (-68.4 %)|95 B|
|`random_smooth("u8", 10000, length=100)`|20 B (-16.7 %)|24 B|
|`random_smooth("u8", 100, length=5000)`|1141 B (-62.2 %)|3020 B|
|`random_smooth("u8", 10000, length=5000)`|632 B (**+1441.5 %**)|41 B|
|`random_smooth("s16", 100, length=100)`|95 B (-57.4 %)|223 B|
|`random_smooth("s16", 10000, length=100)`|24 B (-54.7 %)|53 B|
|`random_smooth("s16", 100, length=5000)`|3793 B (-62.1 %)|10020 B|
|`random_smooth("s16", 10000, length=5000)`|1225 B (-86.6 %)|9108 B|
|`random_smooth("u64", 100, length=100)`|964 B (**+19.5 %**)|807 B|
|`random_smooth("u64", 10000, length=100)`|774 B (**+10.7 %**)|699 B|
|`random_smooth("u64", 100, length=5000)`|33987 B (-9.6 %)|37588 B|
|`random_smooth("u64", 10000, length=5000)`|27159 B (-24.0 %)|35726 B|
|`random_smooth("s64", 100, length=100)`|1001 B (**+21.9 %**)|821 B|
|`random_smooth("s64", 10000, length=100)`|724 B (+4.9 %)|690 B|
|`random_smooth("s64", 100, length=5000)`|34185 B (-9.8 %)|37881 B|
|`random_smooth("s64", 10000, length=5000)`|25309 B (-29.1 %)|35684 B|
|`random_step("u8", 100, length=100)`|20 B (-23.1 %)|26 B|
|`random_step("u8", 10000, length=100)`|20 B (-16.7 %)|24 B|
|`random_step("u8", 100, length=5000)`|695 B (**+292.7 %**)|177 B|
|`random_step("u8", 10000, length=5000)`|632 B (**+1441.5 %**)|41 B|
|`random_step("s16", 100, length=100)`|25 B (-13.8 %)|29 B|
|`random_step("s16", 10000, length=100)`|20 B (-20.0 %)|25 B|
|`random_step("s16", 100, length=5000)`|766 B (**+231.6 %**)|231 B|
|`random_step("s16", 10000, length=5000)`|633 B (**+1218.8 %**)|48 B|
|`random_step("u64", 100, length=100)`|33 B (-8.3 %)|36 B|
|`random_step("u64", 10000, length=100)`|33 B (-8.3 %)|36 B|
|`random_step("u64", 100, length=5000)`|1348 B (**+99.7 %**)|675 B|
|`random_step("u64", 10000, length=5000)`|645 B (**+514.3 %**)|105 B|
|`random_step("s64", 100, length=100)`|32 B (-11.1 %)|36 B|
|`random_step("s64", 10000, length=100)`|32 B (-11.1 %)|36 B|
|`random_step("s64", 100, length=5000)`|1287 B (**+105.9 %**)|625 B|
|`random_step("s64", 10000, length=5000)`|645 B (**+514.3 %**)|105 B|
|`sawtooth("u8", 100, length=100)`|41 B (-66.7 %)|123 B|
|`sawtooth("u8", 10000, length=100)`|19 B (-36.7 %)|30 B|
|`sawtooth("u8", 100, length=5000)`|1854 B (**+800.0 %**)|206 B|
|`sawtooth("u8", 10000, length=5000)`|662 B (**+159.6 %**)|255 B|
|`sawtooth("s16", 100, length=100)`|47 B (-78.9 %)|223 B|
|`sawtooth("s16", 10000, length=100)`|46 B (-75.8 %)|190 B|
|`sawtooth("s16", 100, length=5000)`|1517 B (**+337.2 %**)|347 B|
|`sawtooth("s16", 10000, length=5000)`|1753 B (-82.3 %)|9910 B|
|`sawtooth("u64", 100, length=100)`|95 B (-76.6 %)|406 B|
|`sawtooth("u64", 10000, length=100)`|100 B (-86.6 %)|747 B|
|`sawtooth("u64", 100, length=5000)`|7814 B (**+387.8 %**)|1602 B|
|`sawtooth("u64", 10000, length=5000)`|4594 B (-74.5 %)|17989 B|
|`sawtooth("s64", 100, length=100)`|114 B (-70.5 %)|386 B|
|`sawtooth("s64", 10000, length=100)`|130 B (-82.3 %)|733 B|
|`sawtooth("s64", 100, length=5000)`|7832 B (**+397.6 %**)|1574 B|
|`sawtooth("s64", 10000, length=5000)`|3903 B (-76.5 %)|16599 B|
|`sine("u8", 100, length=100)`|42 B (-65.9 %)|123 B|
|`sine("u8", 10000, length=100)`|22 B (-47.6 %)|42 B|
|`sine("u8", 100, length=5000)`|1659 B (**+782.4 %**)|188 B|
|`sine("u8", 10000, length=5000)`|696 B (**+19.0 %**)|585 B|
|`sine("s16", 100, length=100)`|148 B (-33.6 %)|223 B|
|`sine("s16", 10000, length=100)`|42 B (-81.2 %)|223 B|
|`sine("s16", 100, length=5000)`|6922 B (**+2004.0 %**)|329 B|
|`sine("s16", 10000, length=5000)`|1288 B (-86.3 %)|9430 B|
|`sine("u64", 100, length=100)`|1147 B (**+101.9 %**)|568 B|
|`sine("u64", 10000, length=100)`|709 B (-11.3 %)|799 B|
|`sine("u64", 100, length=5000)`|37313 B (**+195.2 %**)|12642 B|
|`sine("u64", 10000, length=5000)`|28463 B (+2.5 %)|27758 B|
|`sine("s64", 100, length=100)`|1134 B (**+95.9 %**)|579 B|
|`sine("s64", 10000, length=100)`|692 B (-15.9 %)|823 B|
|`sine("s64", 100, length=5000)`|37474 B (**+174.9 %**)|13631 B|
|`sine("s64", 10000, length=5000)`|28446 B (-1.5 %)|28893 B|
|`unexpected_jump("u8", 100, length=100)`|75 B (-8.5 %)|82 B|
|`unexpected_jump("u8", 10000, length=100)`|69 B (-15.9 %)|82 B|
|`unexpected_jump("u8", 100, length=5000)`|3284 B (**+24.9 %**)|2629 B|
|`unexpected_jump("u8", 10000, length=5000)`|3299 B (**+25.3 %**)|2632 B|
|`unexpected_jump("s16", 100, length=100)`|70 B (-38.1 %)|113 B|
|`unexpected_jump("s16", 10000, length=100)`|74 B (-32.1 %)|109 B|
|`unexpected_jump("s16", 100, length=5000)`|3602 B (**+12.1 %**)|3212 B|
|`unexpected_jump("s16", 10000, length=5000)`|3298 B (+4.9 %)|3144 B|
|`unexpected_jump("u64", 100, length=100)`|75 B (-47.6 %)|143 B|
|`unexpected_jump("u64", 10000, length=100)`|79 B (-40.2 %)|132 B|
|`unexpected_jump("u64", 100, length=5000)`|4439 B (**+17.0 %**)|3795 B|
|`unexpected_jump("u64", 10000, length=5000)`|3303 B (-11.1 %)|3716 B|
|`unexpected_jump("s64", 100, length=100)`|110 B (-23.6 %)|144 B|
|`unexpected_jump("s64", 10000, length=100)`|76 B (-45.3 %)|139 B|
|`unexpected_jump("s64", 100, length=5000)`|5047 B (**+32.4 %**)|3813 B|
|`unexpected_jump("s64", 10000, length=5000)`|3332 B (-10.7 %)|3733 B|
|Geometric mean -- CSV datasets|-51.5 %||
|Geometric mean -- all|-21.1 %||

