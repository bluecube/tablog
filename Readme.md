# Tablog

[![CI status](https://github.com/bluecube/tablog/actions/workflows/CI.yml/badge.svg?branch=master)](https://github.com/bluecube/tablog/actions/workflows/CI.yml)
[![Coverage Status](https://coveralls.io/repos/github/bluecube/tablog/badge.svg?branch=master)](https://coveralls.io/github/bluecube/tablog?branch=master)

A C++17 library for writing and reading compressed tables of (prefereably
continuous) numerical values, for example logging streams of sensor data on microcontrollers.

## Goals

1. Fast compression with small memory footprint
2. Decent compression ratio (hopefully better than GZipped CSV)
3. Streamable (decoding can be picked up from the midlle of the stream, after reading limited amount of data)
4. Should work well on ESP32, ARM (Raspberry PI), x64 (mostly for development and testing) and (with lower priority) AVR.

## Compression ratio overview

|Dataset|Tablog: Compressed size|Gzip: Compressed size|
|-------|-----------------------|---------------------|
|`dataset1/drift.csv`|19409 B (-57.5 %)|45646 B|
|`dataset1/power.csv`|449125 B (-16.8 %)|539491 B|
|`dataset1/soc.csv`|22852 B (-16.6 %)|27394 B|
|`dataset1/tph.csv`|23100 B (-43.5 %)|40877 B|
|`gcode/parts.csv`|255445 B (-34.3 %)|388618 B|
|`phone_imu/imu.csv`|174087 B (-32.4 %)|257485 B|
|`phone_imu/magnetometer.csv`|51790 B (-48.0 %)|99575 B|
|`twinsen/18e755d7ecc2b2a3_data.csv`|390038 B (-68.0 %)|1218594 B|
|`twinsen/66d8276d3a2480c2_data.csv`|550317 B (-60.9 %)|1407575 B|
|`empty("u8")`|5 B (-75.0 %)|20 B|
|`empty("s16")`|5 B (-75.0 %)|20 B|
|`empty("u64")`|5 B (-75.0 %)|20 B|
|`empty("s64")`|5 B (-75.0 %)|20 B|
|`count_up("u8", length=100)`|18 B (-85.0 %)|120 B|
|`count_up("u8", length=5000)`|680 B (**+103.0 %**)|335 B|
|`count_up("s16", length=100)`|28 B (-82.2 %)|157 B|
|`count_up("s16", length=5000)`|640 B (-92.7 %)|8790 B|
|`count_up("u64", length=100)`|20 B (-89.3 %)|187 B|
|`count_up("u64", length=5000)`|633 B (-93.9 %)|10353 B|
|`count_up("s64", length=100)`|57 B (-69.7 %)|188 B|
|`count_up("s64", length=5000)`|670 B (-93.5 %)|10328 B|
|`minor7chord("u8", 100, length=100)`|49 B (-58.8 %)|119 B|
|`minor7chord("u8", 10000, length=100)`|30 B (-33.3 %)|45 B|
|`minor7chord("u8", 100, length=5000)`|1319 B (**+28.4 %**)|1027 B|
|`minor7chord("u8", 10000, length=5000)`|724 B (-1.1 %)|732 B|
|`minor7chord("s16", 100, length=100)`|149 B (-33.2 %)|223 B|
|`minor7chord("s16", 10000, length=100)`|36 B (-83.9 %)|223 B|
|`minor7chord("s16", 100, length=5000)`|6263 B (**+187.8 %**)|2176 B|
|`minor7chord("s16", 10000, length=5000)`|1265 B (-87.1 %)|9833 B|
|`minor7chord("u64", 100, length=100)`|757 B (-6.8 %)|812 B|
|`minor7chord("u64", 10000, length=100)`|691 B (-15.2 %)|815 B|
|`minor7chord("u64", 100, length=5000)`|36287 B (**+71.5 %**)|21154 B|
|`minor7chord("u64", 10000, length=5000)`|28937 B (-21.4 %)|36832 B|
|`minor7chord("s64", 100, length=100)`|749 B (-9.0 %)|823 B|
|`minor7chord("s64", 10000, length=100)`|683 B (-17.0 %)|823 B|
|`minor7chord("s64", 100, length=5000)`|36279 B (**+65.2 %**)|21966 B|
|`minor7chord("s64", 10000, length=5000)`|28927 B (-22.7 %)|37426 B|
|`random("u8", length=100)`|135 B (+9.8 %)|123 B|
|`random("u8", length=5000)`|6403 B (**+27.5 %**)|5023 B|
|`random("s16", length=100)`|235 B (+5.4 %)|223 B|
|`random("s16", length=5000)`|11424 B (**+14.0 %**)|10023 B|
|`random("u64", length=100)`|844 B (+2.6 %)|823 B|
|`random("u64", length=5000)`|41455 B (+3.6 %)|40033 B|
|`random("s64", length=100)`|840 B (+2.1 %)|823 B|
|`random("s64", length=5000)`|41446 B (+3.5 %)|40033 B|
|`random_smooth("u8", 100, length=100)`|30 B (-68.4 %)|95 B|
|`random_smooth("u8", 10000, length=100)`|20 B (-16.7 %)|24 B|
|`random_smooth("u8", 100, length=5000)`|1141 B (-62.2 %)|3020 B|
|`random_smooth("u8", 10000, length=5000)`|632 B (**+1441.5 %**)|41 B|
|`random_smooth("s16", 100, length=100)`|126 B (-43.5 %)|223 B|
|`random_smooth("s16", 10000, length=100)`|43 B (-18.9 %)|53 B|
|`random_smooth("s16", 100, length=5000)`|3818 B (-61.9 %)|10020 B|
|`random_smooth("s16", 10000, length=5000)`|1284 B (-85.9 %)|9108 B|
|`random_smooth("u64", 100, length=100)`|706 B (-12.5 %)|807 B|
|`random_smooth("u64", 10000, length=100)`|691 B (-1.1 %)|699 B|
|`random_smooth("u64", 100, length=5000)`|33704 B (-10.3 %)|37588 B|
|`random_smooth("u64", 10000, length=5000)`|27070 B (-24.2 %)|35726 B|
|`random_smooth("s64", 100, length=100)`|719 B (-12.4 %)|821 B|
|`random_smooth("s64", 10000, length=100)`|690 B (+0.0 %)|690 B|
|`random_smooth("s64", 100, length=5000)`|33849 B (-10.6 %)|37881 B|
|`random_smooth("s64", 10000, length=5000)`|25272 B (-29.2 %)|35684 B|
|`random_step("u8", 100, length=100)`|20 B (-23.1 %)|26 B|
|`random_step("u8", 10000, length=100)`|20 B (-16.7 %)|24 B|
|`random_step("u8", 100, length=5000)`|691 B (**+290.4 %**)|177 B|
|`random_step("u8", 10000, length=5000)`|632 B (**+1441.5 %**)|41 B|
|`random_step("s16", 100, length=100)`|24 B (-17.2 %)|29 B|
|`random_step("s16", 10000, length=100)`|20 B (-20.0 %)|25 B|
|`random_step("s16", 100, length=5000)`|737 B (**+219.0 %**)|231 B|
|`random_step("s16", 10000, length=5000)`|633 B (**+1218.8 %**)|48 B|
|`random_step("u64", 100, length=100)`|33 B (-8.3 %)|36 B|
|`random_step("u64", 10000, length=100)`|33 B (-8.3 %)|36 B|
|`random_step("u64", 100, length=5000)`|1060 B (**+57.0 %**)|675 B|
|`random_step("u64", 10000, length=5000)`|645 B (**+514.3 %**)|105 B|
|`random_step("s64", 100, length=100)`|32 B (-11.1 %)|36 B|
|`random_step("s64", 10000, length=100)`|32 B (-11.1 %)|36 B|
|`random_step("s64", 100, length=5000)`|1021 B (**+63.4 %**)|625 B|
|`random_step("s64", 10000, length=5000)`|645 B (**+514.3 %**)|105 B|
|`sawtooth("u8", 100, length=100)`|41 B (-66.7 %)|123 B|
|`sawtooth("u8", 10000, length=100)`|19 B (-36.7 %)|30 B|
|`sawtooth("u8", 100, length=5000)`|2540 B (**+1133.0 %**)|206 B|
|`sawtooth("u8", 10000, length=5000)`|662 B (**+159.6 %**)|255 B|
|`sawtooth("s16", 100, length=100)`|99 B (-55.6 %)|223 B|
|`sawtooth("s16", 10000, length=100)`|104 B (-45.3 %)|190 B|
|`sawtooth("s16", 100, length=5000)`|1569 B (**+352.2 %**)|347 B|
|`sawtooth("s16", 10000, length=5000)`|1810 B (-81.7 %)|9910 B|
|`sawtooth("u64", 100, length=100)`|284 B (-30.0 %)|406 B|
|`sawtooth("u64", 10000, length=100)`|496 B (-33.6 %)|747 B|
|`sawtooth("u64", 100, length=5000)`|8426 B (**+426.0 %**)|1602 B|
|`sawtooth("u64", 10000, length=5000)`|5389 B (-70.0 %)|17989 B|
|`sawtooth("s64", 100, length=100)`|242 B (-37.3 %)|386 B|
|`sawtooth("s64", 10000, length=100)`|342 B (-53.3 %)|733 B|
|`sawtooth("s64", 100, length=5000)`|8416 B (**+434.7 %**)|1574 B|
|`sawtooth("s64", 10000, length=5000)`|4561 B (-72.5 %)|16599 B|
|`sine("u8", 100, length=100)`|75 B (-39.0 %)|123 B|
|`sine("u8", 10000, length=100)`|27 B (-35.7 %)|42 B|
|`sine("u8", 100, length=5000)`|3039 B (**+1516.5 %**)|188 B|
|`sine("u8", 10000, length=5000)`|710 B (**+21.4 %**)|585 B|
|`sine("s16", 100, length=100)`|191 B (-14.3 %)|223 B|
|`sine("s16", 10000, length=100)`|43 B (-80.7 %)|223 B|
|`sine("s16", 100, length=5000)`|8668 B (**+2534.7 %**)|329 B|
|`sine("s16", 10000, length=5000)`|1290 B (-86.3 %)|9430 B|
|`sine("u64", 100, length=100)`|799 B (**+40.7 %**)|568 B|
|`sine("u64", 10000, length=100)`|690 B (-13.6 %)|799 B|
|`sine("u64", 100, length=5000)`|38487 B (**+204.4 %**)|12642 B|
|`sine("u64", 10000, length=5000)`|28540 B (+2.8 %)|27758 B|
|`sine("s64", 100, length=100)`|790 B (**+36.4 %**)|579 B|
|`sine("s64", 10000, length=100)`|682 B (-17.1 %)|823 B|
|`sine("s64", 100, length=5000)`|38667 B (**+183.7 %**)|13631 B|
|`sine("s64", 10000, length=5000)`|28532 B (-1.2 %)|28893 B|
|`unexpected_jump("u8", 100, length=100)`|80 B (-2.4 %)|82 B|
|`unexpected_jump("u8", 10000, length=100)`|70 B (-14.6 %)|82 B|
|`unexpected_jump("u8", 100, length=5000)`|3288 B (**+25.1 %**)|2629 B|
|`unexpected_jump("u8", 10000, length=5000)`|3311 B (**+25.8 %**)|2632 B|
|`unexpected_jump("s16", 100, length=100)`|70 B (-38.1 %)|113 B|
|`unexpected_jump("s16", 10000, length=100)`|74 B (-32.1 %)|109 B|
|`unexpected_jump("s16", 100, length=5000)`|5090 B (**+58.5 %**)|3212 B|
|`unexpected_jump("s16", 10000, length=5000)`|3292 B (+4.7 %)|3144 B|
|`unexpected_jump("u64", 100, length=100)`|75 B (-47.6 %)|143 B|
|`unexpected_jump("u64", 10000, length=100)`|79 B (-40.2 %)|132 B|
|`unexpected_jump("u64", 100, length=5000)`|21291 B (**+461.0 %**)|3795 B|
|`unexpected_jump("u64", 10000, length=5000)`|3298 B (-11.2 %)|3716 B|
|`unexpected_jump("s64", 100, length=100)`|213 B (**+47.9 %**)|144 B|
|`unexpected_jump("s64", 10000, length=100)`|75 B (-46.0 %)|139 B|
|`unexpected_jump("s64", 100, length=5000)`|26895 B (**+605.4 %**)|3813 B|
|`unexpected_jump("s64", 10000, length=5000)`|3324 B (-11.0 %)|3733 B|
|Geometric mean|-12.8 %||

