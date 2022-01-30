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

|Dataset|Tablog: Compressed size|Gzip: Compressed size|Gzip Δ: Compressed size|
|-------|-----------------------|---------------------|-----------------------|
|`dataset1/drift.csv`|19409 B (**+52.7 %**)|45646 B|12710 B|
|`dataset1/power.csv`|449125 B (-12.6 %)|539491 B|513621 B|
|`dataset1/soc.csv`|22852 B (**+14.8 %**)|27394 B|19913 B|
|`dataset1/tph.csv`|23100 B (-24.8 %)|40877 B|30733 B|
|`gcode/parts.csv`|255445 B (+6.1 %)|388618 B|240736 B|
|`phone_imu/imu.csv`|174087 B (+4.7 %)|257485 B|166199 B|
|`phone_imu/magnetometer.csv`|51790 B (+7.6 %)|99575 B|48138 B|
|`twinsen/18e755d7ecc2b2a3_data.csv`|390038 B (-14.0 %)|1218594 B|453517 B|
|`twinsen/66d8276d3a2480c2_data.csv`|550317 B (-16.4 %)|1407575 B|658629 B|
|`empty("u8")`|5 B (-75.0 %)|20 B|20 B|
|`empty("s16")`|5 B (-75.0 %)|20 B|20 B|
|`empty("u64")`|5 B (-75.0 %)|20 B|20 B|
|`empty("s64")`|5 B (-75.0 %)|20 B|20 B|
|`count_up("u8", length=100)`|18 B (-25.0 %)|120 B|24 B|
|`count_up("u8", length=5000)`|680 B (**+1093.0 %**)|335 B|57 B|
|`count_up("s16", length=100)`|28 B (+7.7 %)|157 B|26 B|
|`count_up("s16", length=5000)`|640 B (**+1261.7 %**)|8790 B|47 B|
|`count_up("u64", length=100)`|20 B (-35.5 %)|187 B|31 B|
|`count_up("u64", length=5000)`|633 B (**+559.4 %**)|10353 B|96 B|
|`count_up("s64", length=100)`|57 B (**+83.9 %**)|188 B|31 B|
|`count_up("s64", length=5000)`|670 B (**+597.9 %**)|10328 B|96 B|
|`minor7chord("u8", 100, length=100)`|49 B (-43.7 %)|119 B|87 B|
|`minor7chord("u8", 10000, length=100)`|30 B (**+15.4 %**)|45 B|26 B|
|`minor7chord("u8", 100, length=5000)`|1319 B (**+175.9 %**)|1027 B|478 B|
|`minor7chord("u8", 10000, length=5000)`|724 B (**+256.7 %**)|732 B|203 B|
|`minor7chord("s16", 100, length=100)`|149 B (-33.2 %)|223 B|223 B|
|`minor7chord("s16", 10000, length=100)`|36 B (+0.0 %)|223 B|36 B|
|`minor7chord("s16", 100, length=5000)`|6263 B (**+236.9 %**)|2176 B|1859 B|
|`minor7chord("s16", 10000, length=5000)`|1265 B (**+34.7 %**)|9833 B|939 B|
|`minor7chord("u64", 100, length=100)`|757 B (-6.4 %)|812 B|809 B|
|`minor7chord("u64", 10000, length=100)`|691 B (**+10.7 %**)|815 B|624 B|
|`minor7chord("u64", 100, length=5000)`|36287 B (**+90.6 %**)|21154 B|19034 B|
|`minor7chord("u64", 10000, length=5000)`|28937 B (-7.5 %)|36832 B|31269 B|
|`minor7chord("s64", 100, length=100)`|749 B (-7.5 %)|823 B|810 B|
|`minor7chord("s64", 10000, length=100)`|683 B (-0.6 %)|823 B|687 B|
|`minor7chord("s64", 100, length=5000)`|36279 B (**+81.0 %**)|21966 B|20040 B|
|`minor7chord("s64", 10000, length=5000)`|28927 B (-11.6 %)|37426 B|32722 B|
|`random("u8", length=100)`|135 B (+9.8 %)|123 B|123 B|
|`random("u8", length=5000)`|6403 B (**+27.5 %**)|5023 B|5023 B|
|`random("s16", length=100)`|235 B (+5.4 %)|223 B|223 B|
|`random("s16", length=5000)`|11424 B (**+14.0 %**)|10023 B|10023 B|
|`random("u64", length=100)`|844 B (+2.6 %)|823 B|823 B|
|`random("u64", length=5000)`|41455 B (+3.6 %)|40033 B|40033 B|
|`random("s64", length=100)`|840 B (+2.1 %)|823 B|823 B|
|`random("s64", length=5000)`|41446 B (+3.5 %)|40033 B|40033 B|
|`random_smooth("u8", 100, length=100)`|30 B (-50.0 %)|95 B|60 B|
|`random_smooth("u8", 10000, length=100)`|20 B (-16.7 %)|24 B|24 B|
|`random_smooth("u8", 100, length=5000)`|1141 B (**+27.3 %**)|3020 B|896 B|
|`random_smooth("u8", 10000, length=5000)`|632 B (**+1441.5 %**)|41 B|41 B|
|`random_smooth("s16", 100, length=100)`|126 B (-32.3 %)|223 B|186 B|
|`random_smooth("s16", 10000, length=100)`|43 B (+4.9 %)|53 B|41 B|
|`random_smooth("s16", 100, length=5000)`|3818 B (-43.1 %)|10020 B|6711 B|
|`random_smooth("s16", 10000, length=5000)`|1284 B (**+176.7 %**)|9108 B|464 B|
|`random_smooth("u64", 100, length=100)`|706 B (+8.3 %)|807 B|652 B|
|`random_smooth("u64", 10000, length=100)`|691 B (+4.2 %)|699 B|663 B|
|`random_smooth("u64", 100, length=5000)`|33704 B (**+38.1 %**)|37588 B|24410 B|
|`random_smooth("u64", 10000, length=5000)`|27070 B (**+14.4 %**)|35726 B|23656 B|
|`random_smooth("s64", 100, length=100)`|719 B (**+14.1 %**)|821 B|630 B|
|`random_smooth("s64", 10000, length=100)`|690 B (**+13.5 %**)|690 B|608 B|
|`random_smooth("s64", 100, length=5000)`|33849 B (**+33.3 %**)|37881 B|25385 B|
|`random_smooth("s64", 10000, length=5000)`|25272 B (-4.3 %)|35684 B|26403 B|
|`random_step("u8", 100, length=100)`|20 B (-25.9 %)|26 B|27 B|
|`random_step("u8", 10000, length=100)`|20 B (-16.7 %)|24 B|24 B|
|`random_step("u8", 100, length=5000)`|691 B (**+233.8 %**)|177 B|207 B|
|`random_step("u8", 10000, length=5000)`|632 B (**+1441.5 %**)|41 B|41 B|
|`random_step("s16", 100, length=100)`|24 B (-17.2 %)|29 B|29 B|
|`random_step("s16", 10000, length=100)`|20 B (-20.0 %)|25 B|25 B|
|`random_step("s16", 100, length=5000)`|737 B (**+169.0 %**)|231 B|274 B|
|`random_step("s16", 10000, length=5000)`|633 B (**+1218.8 %**)|48 B|48 B|
|`random_step("u64", 100, length=100)`|33 B (-8.3 %)|36 B|36 B|
|`random_step("u64", 10000, length=100)`|33 B (-8.3 %)|36 B|36 B|
|`random_step("u64", 100, length=5000)`|1060 B (**+48.9 %**)|675 B|712 B|
|`random_step("u64", 10000, length=5000)`|645 B (**+650.0 %**)|105 B|86 B|
|`random_step("s64", 100, length=100)`|32 B (-11.1 %)|36 B|36 B|
|`random_step("s64", 10000, length=100)`|32 B (-11.1 %)|36 B|36 B|
|`random_step("s64", 100, length=5000)`|1021 B (**+54.9 %**)|625 B|659 B|
|`random_step("s64", 10000, length=5000)`|645 B (**+650.0 %**)|105 B|86 B|
|`sawtooth("u8", 100, length=100)`|41 B (**+28.1 %**)|123 B|32 B|
|`sawtooth("u8", 10000, length=100)`|19 B (-40.6 %)|30 B|32 B|
|`sawtooth("u8", 100, length=5000)`|2540 B (**+2230.3 %**)|206 B|109 B|
|`sawtooth("u8", 10000, length=5000)`|662 B (**+782.7 %**)|255 B|75 B|
|`sawtooth("s16", 100, length=100)`|99 B (**+160.5 %**)|223 B|38 B|
|`sawtooth("s16", 10000, length=100)`|104 B (**+181.1 %**)|190 B|37 B|
|`sawtooth("s16", 100, length=5000)`|1569 B (**+1045.3 %**)|347 B|137 B|
|`sawtooth("s16", 10000, length=5000)`|1810 B (**+1674.5 %**)|9910 B|102 B|
|`sawtooth("u64", 100, length=100)`|284 B (**+289.0 %**)|406 B|73 B|
|`sawtooth("u64", 10000, length=100)`|496 B (**+561.3 %**)|747 B|75 B|
|`sawtooth("u64", 100, length=5000)`|8426 B (**+2476.8 %**)|1602 B|327 B|
|`sawtooth("u64", 10000, length=5000)`|5389 B (**+1573.6 %**)|17989 B|322 B|
|`sawtooth("s64", 100, length=100)`|242 B (**+296.7 %**)|386 B|61 B|
|`sawtooth("s64", 10000, length=100)`|342 B (**+627.7 %**)|733 B|47 B|
|`sawtooth("s64", 100, length=5000)`|8416 B (**+2580.3 %**)|1574 B|314 B|
|`sawtooth("s64", 10000, length=5000)`|4561 B (**+1601.9 %**)|16599 B|268 B|
|`sine("u8", 100, length=100)`|75 B (-16.7 %)|123 B|90 B|
|`sine("u8", 10000, length=100)`|27 B (-6.9 %)|42 B|29 B|
|`sine("u8", 100, length=5000)`|3039 B (**+1926.0 %**)|188 B|150 B|
|`sine("u8", 10000, length=5000)`|710 B (**+301.1 %**)|585 B|177 B|
|`sine("s16", 100, length=100)`|191 B (-9.5 %)|223 B|211 B|
|`sine("s16", 10000, length=100)`|43 B (+7.5 %)|223 B|40 B|
|`sine("s16", 100, length=5000)`|8668 B (**+2742.0 %**)|329 B|305 B|
|`sine("s16", 10000, length=5000)`|1290 B (**+75.0 %**)|9430 B|737 B|
|`sine("u64", 100, length=100)`|799 B (**+34.5 %**)|568 B|594 B|
|`sine("u64", 10000, length=100)`|690 B (**+15.0 %**)|799 B|600 B|
|`sine("u64", 100, length=5000)`|38487 B (**+330.5 %**)|12642 B|8940 B|
|`sine("u64", 10000, length=5000)`|28540 B (-6.2 %)|27758 B|30427 B|
|`sine("s64", 100, length=100)`|790 B (**+30.4 %**)|579 B|606 B|
|`sine("s64", 10000, length=100)`|682 B (+2.1 %)|823 B|668 B|
|`sine("s64", 100, length=5000)`|38667 B (**+262.7 %**)|13631 B|10662 B|
|`sine("s64", 10000, length=5000)`|28532 B (-10.6 %)|28893 B|31914 B|
|`unexpected_jump("u8", 100, length=100)`|80 B (-12.1 %)|82 B|91 B|
|`unexpected_jump("u8", 10000, length=100)`|70 B (-23.9 %)|82 B|92 B|
|`unexpected_jump("u8", 100, length=5000)`|3288 B (**+12.8 %**)|2629 B|2915 B|
|`unexpected_jump("u8", 10000, length=5000)`|3311 B (**+13.5 %**)|2632 B|2917 B|
|`unexpected_jump("s16", 100, length=100)`|70 B (-43.1 %)|113 B|123 B|
|`unexpected_jump("s16", 10000, length=100)`|74 B (-43.1 %)|109 B|130 B|
|`unexpected_jump("s16", 100, length=5000)`|5090 B (**+32.5 %**)|3212 B|3841 B|
|`unexpected_jump("s16", 10000, length=5000)`|3292 B (-11.0 %)|3144 B|3698 B|
|`unexpected_jump("u64", 100, length=100)`|75 B (-55.4 %)|143 B|168 B|
|`unexpected_jump("u64", 10000, length=100)`|79 B (-50.6 %)|132 B|160 B|
|`unexpected_jump("u64", 100, length=5000)`|21291 B (**+367.9 %**)|3795 B|4550 B|
|`unexpected_jump("u64", 10000, length=5000)`|3298 B (-25.0 %)|3716 B|4396 B|
|`unexpected_jump("s64", 100, length=100)`|213 B (**+21.7 %**)|144 B|175 B|
|`unexpected_jump("s64", 10000, length=100)`|75 B (-54.0 %)|139 B|163 B|
|`unexpected_jump("s64", 100, length=5000)`|26895 B (**+481.3 %**)|3813 B|4627 B|
|`unexpected_jump("s64", 10000, length=5000)`|3324 B (-24.7 %)|3733 B|4416 B|
|Geometric mean|**+67.2 %**|||

