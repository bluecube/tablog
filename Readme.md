# Tablog

[![Stability: Experimental](https://masterminds.github.io/stability/experimental.svg)](https://masterminds.github.io/stability/experimental.html)
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

|Dataset|Tablog|Gzip CSV|Gzip binary|Gzip binary Δ|
|-------|------|--------|-----------|-------------|
`dataset1/drift.csv`|14.8 kB|49.1 kB (+231.9 %)|45.6 kB (+208.5 %)|12.7 kB (**-14.1 %**)|
`dataset1/power.csv`|449 kB|636 kB (+41.6 %)|539 kB (+20.2 %)|514 kB (+14.4 %)|
`dataset1/soc.csv`|19.2 kB|34.5 kB (+79.5 %)|27.4 kB (+42.6 %)|19.9 kB (+3.7 %)|
`dataset1/tph.csv`|21.9 kB|48.6 kB (+121.6 %)|40.9 kB (+86.4 %)|30.7 kB (+40.2 %)|
`gcode/parts.csv`|202 kB|415 kB (+105.4 %)|389 kB (+92.1 %)|241 kB (+19.0 %)|
`phone_imu/imu.csv`|153 kB|271 kB (+76.6 %)|257 kB (+68.0 %)|166 kB (+8.5 %)|
`phone_imu/magnetometer.csv`|50 kB|113 kB (+126.2 %)|99.6 kB (+99.3 %)|48.1 kB (-3.6 %)|
`twinsen/18e755d7ecc2b2a3_data.csv`|321 kB|1.38 MB (+330.5 %)|1.22 MB (+279.9 %)|454 kB (+41.4 %)|
`twinsen/66d8276d3a2480c2_data.csv`|504 kB|1.56 MB (+208.6 %)|1.41 MB (+179.2 %)|659 kB (+30.7 %)|
`empty("u8")`|5 B|33 B (+560.0 %)|20 B (+300.0 %)|20 B (+300.0 %)|
`empty("s16")`|5 B|33 B (+560.0 %)|20 B (+300.0 %)|20 B (+300.0 %)|
`empty("u64")`|5 B|33 B (+560.0 %)|20 B (+300.0 %)|20 B (+300.0 %)|
`empty("s64")`|5 B|33 B (+560.0 %)|20 B (+300.0 %)|20 B (+300.0 %)|
`count_up("u8", length=100)`|18 B|201 B (+1016.7 %)|120 B (+566.7 %)|24 B (+33.3 %)|
`count_up("u8", length=5000)`|674 B|680 B (+0.9 %)|335 B (**-50.3 %**)|57 B (**-91.5 %**)|
`count_up("s16", length=100)`|23 B|200 B (+769.6 %)|157 B (+582.6 %)|26 B (+13.0 %)|
`count_up("s16", length=5000)`|635 B|10 kB (+1478.0 %)|8.79 kB (+1284.3 %)|47 B (**-92.6 %**)|
`count_up("u64", length=100)`|20 B|201 B (+905.0 %)|187 B (+835.0 %)|31 B (+55.0 %)|
`count_up("u64", length=5000)`|633 B|11.2 kB (+1668.4 %)|10.4 kB (+1535.5 %)|96 B (**-84.8 %**)|
`count_up("s64", length=100)`|36 B|245 B (+580.6 %)|188 B (+422.2 %)|31 B (**-13.9 %**)|
`count_up("s64", length=5000)`|649 B|12.6 kB (+1848.5 %)|10.3 kB (+1491.4 %)|96 B (**-85.2 %**)|
`minor7chord("u8", 100, length=100)`|37 B|207 B (+459.5 %)|119 B (+221.6 %)|87 B (+135.1 %)|
`minor7chord("u8", 10000, length=100)`|22 B|69 B (+213.6 %)|45 B (+104.5 %)|26 B (+18.2 %)|
`minor7chord("u8", 100, length=5000)`|1.31 kB|1.52 kB (+16.4 %)|1.03 kB (**-21.4 %**)|478 B (**-63.4 %**)|
`minor7chord("u8", 10000, length=5000)`|710 B|688 B (-3.1 %)|732 B (+3.1 %)|203 B (**-71.4 %**)|
`minor7chord("s16", 100, length=100)`|141 B|331 B (+134.8 %)|223 B (+58.2 %)|223 B (+58.2 %)|
`minor7chord("s16", 10000, length=100)`|34 B|286 B (+741.2 %)|223 B (+555.9 %)|36 B (+5.9 %)|
`minor7chord("s16", 100, length=5000)`|6.26 kB|3.19 kB (**-49.0 %**)|2.18 kB (**-65.2 %**)|1.86 kB (**-70.3 %**)|
`minor7chord("s16", 10000, length=5000)`|1.26 kB|13.1 kB (+938.8 %)|9.83 kB (+678.5 %)|939 B (**-25.7 %**)|
`minor7chord("u64", 100, length=100)`|1.14 kB|1.08 kB (-4.7 %)|812 B (**-28.7 %**)|809 B (**-29.0 %**)|
`minor7chord("u64", 10000, length=100)`|747 B|1.04 kB (+39.8 %)|815 B (+9.1 %)|624 B (**-16.5 %**)|
`minor7chord("u64", 100, length=5000)`|36.8 kB|33.1 kB (**-10.0 %**)|21.2 kB (**-42.5 %**)|19 kB (**-48.2 %**)|
`minor7chord("u64", 10000, length=5000)`|29 kB|48.8 kB (+68.5 %)|36.8 kB (+27.0 %)|31.3 kB (+7.8 %)|
`minor7chord("s64", 100, length=100)`|1.13 kB|1.07 kB (-4.6 %)|823 B (**-26.9 %**)|810 B (**-28.1 %**)|
`minor7chord("s64", 10000, length=100)`|730 B|1.02 kB (+39.7 %)|823 B (+12.7 %)|687 B (-5.9 %)|
`minor7chord("s64", 100, length=5000)`|36.7 kB|30.5 kB (**-17.0 %**)|22 kB (**-40.2 %**)|20 kB (**-45.5 %**)|
`minor7chord("s64", 10000, length=5000)`|29 kB|48.6 kB (+67.9 %)|37.4 kB (+29.1 %)|32.7 kB (+12.9 %)|
`random("u8", length=100)`|138 B|224 B (+62.3 %)|123 B (**-10.9 %**)|123 B (**-10.9 %**)|
`random("u8", length=5000)`|6.41 kB|7.42 kB (+15.8 %)|5.02 kB (**-21.6 %**)|5.02 kB (**-21.6 %**)|
`random("s16", length=100)`|260 B|369 B (+41.9 %)|223 B (**-14.2 %**)|223 B (**-14.2 %**)|
`random("s16", length=5000)`|11.5 kB|14.6 kB (+27.9 %)|10 kB (**-12.5 %**)|10 kB (**-12.5 %**)|
`random("u64", length=100)`|1.34 kB|1.11 kB (**-16.8 %**)|823 B (**-38.4 %**)|823 B (**-38.4 %**)|
`random("u64", length=5000)`|42.1 kB|51.3 kB (+21.8 %)|40 kB (-5.0 %)|40 kB (-5.0 %)|
`random("s64", length=100)`|1.33 kB|1.12 kB (**-15.4 %**)|823 B (**-38.0 %**)|823 B (**-38.0 %**)|
`random("s64", length=5000)`|42.1 kB|51.4 kB (+22.1 %)|40 kB (-4.9 %)|40 kB (-4.9 %)|
`random_smooth("u8", 100, length=100)`|30 B|167 B (+456.7 %)|95 B (+216.7 %)|60 B (+100.0 %)|
`random_smooth("u8", 10000, length=100)`|20 B|40 B (+100.0 %)|24 B (+20.0 %)|24 B (+20.0 %)|
`random_smooth("u8", 100, length=5000)`|1.14 kB|3.62 kB (+217.3 %)|3.02 kB (+164.7 %)|896 B (**-21.5 %**)|
`random_smooth("u8", 10000, length=5000)`|632 B|92 B (**-85.4 %**)|41 B (**-93.5 %**)|41 B (**-93.5 %**)|
`random_smooth("s16", 100, length=100)`|95 B|332 B (+249.5 %)|223 B (+134.7 %)|186 B (+95.8 %)|
`random_smooth("s16", 10000, length=100)`|24 B|74 B (+208.3 %)|53 B (+120.8 %)|41 B (+70.8 %)|
`random_smooth("s16", 100, length=5000)`|3.79 kB|14 kB (+270.4 %)|10 kB (+164.2 %)|6.71 kB (+76.9 %)|
`random_smooth("s16", 10000, length=5000)`|1.23 kB|12 kB (+882.8 %)|9.11 kB (+643.5 %)|464 B (**-62.1 %**)|
`random_smooth("u64", 100, length=100)`|964 B|1.05 kB (+9.2 %)|807 B (**-16.3 %**)|652 B (**-32.4 %**)|
`random_smooth("u64", 10000, length=100)`|774 B|949 B (+22.6 %)|699 B (-9.7 %)|663 B (**-14.3 %**)|
`random_smooth("u64", 100, length=5000)`|34 kB|49.4 kB (+45.4 %)|37.6 kB (+10.6 %)|24.4 kB (**-28.2 %**)|
`random_smooth("u64", 10000, length=5000)`|27.2 kB|48 kB (+76.9 %)|35.7 kB (+31.5 %)|23.7 kB (**-12.9 %**)|
`random_smooth("s64", 100, length=100)`|1 kB|1.05 kB (+5.3 %)|821 B (**-18.0 %**)|630 B (**-37.1 %**)|
`random_smooth("s64", 10000, length=100)`|724 B|898 B (+24.0 %)|690 B (-4.7 %)|608 B (**-16.0 %**)|
`random_smooth("s64", 100, length=5000)`|34.2 kB|49.5 kB (+44.8 %)|37.9 kB (+10.8 %)|25.4 kB (**-25.7 %**)|
`random_smooth("s64", 10000, length=5000)`|25.3 kB|46.1 kB (+82.1 %)|35.7 kB (+41.0 %)|26.4 kB (+4.3 %)|
`random_step("u8", 100, length=100)`|20 B|42 B (+110.0 %)|26 B (+30.0 %)|27 B (+35.0 %)|
`random_step("u8", 10000, length=100)`|20 B|40 B (+100.0 %)|24 B (+20.0 %)|24 B (+20.0 %)|
`random_step("u8", 100, length=5000)`|695 B|252 B (**-63.7 %**)|177 B (**-74.5 %**)|207 B (**-70.2 %**)|
`random_step("u8", 10000, length=5000)`|632 B|92 B (**-85.4 %**)|41 B (**-93.5 %**)|41 B (**-93.5 %**)|
`random_step("s16", 100, length=100)`|25 B|51 B (+104.0 %)|29 B (+16.0 %)|29 B (+16.0 %)|
`random_step("s16", 10000, length=100)`|20 B|43 B (+115.0 %)|25 B (+25.0 %)|25 B (+25.0 %)|
`random_step("s16", 100, length=5000)`|766 B|348 B (**-54.6 %**)|231 B (**-69.8 %**)|274 B (**-64.2 %**)|
`random_step("s16", 10000, length=5000)`|633 B|100 B (**-84.2 %**)|48 B (**-92.4 %**)|48 B (**-92.4 %**)|
`random_step("u64", 100, length=100)`|33 B|72 B (+118.2 %)|36 B (+9.1 %)|36 B (+9.1 %)|
`random_step("u64", 10000, length=100)`|33 B|72 B (+118.2 %)|36 B (+9.1 %)|36 B (+9.1 %)|
`random_step("u64", 100, length=5000)`|1.35 kB|996 B (**-26.1 %**)|675 B (**-49.9 %**)|712 B (**-47.2 %**)|
`random_step("u64", 10000, length=5000)`|645 B|336 B (**-47.9 %**)|105 B (**-83.7 %**)|86 B (**-86.7 %**)|
`random_step("s64", 100, length=100)`|32 B|67 B (+109.4 %)|36 B (+12.5 %)|36 B (+12.5 %)|
`random_step("s64", 10000, length=100)`|32 B|70 B (+118.8 %)|36 B (+12.5 %)|36 B (+12.5 %)|
`random_step("s64", 100, length=5000)`|1.29 kB|934 B (**-27.4 %**)|625 B (**-51.4 %**)|659 B (**-48.8 %**)|
`random_step("s64", 10000, length=5000)`|645 B|322 B (**-50.1 %**)|105 B (**-83.7 %**)|86 B (**-86.7 %**)|
`sawtooth("u8", 100, length=100)`|41 B|215 B (+424.4 %)|123 B (+200.0 %)|32 B (**-22.0 %**)|
`sawtooth("u8", 10000, length=100)`|19 B|45 B (+136.8 %)|30 B (+57.9 %)|32 B (+68.4 %)|
`sawtooth("u8", 100, length=5000)`|1.85 kB|389 B (**-79.0 %**)|206 B (**-88.9 %**)|109 B (**-94.1 %**)|
`sawtooth("u8", 10000, length=5000)`|662 B|289 B (**-56.3 %**)|255 B (**-61.5 %**)|75 B (**-88.7 %**)|
`sawtooth("s16", 100, length=100)`|47 B|345 B (+634.0 %)|223 B (+374.5 %)|38 B (**-19.1 %**)|
`sawtooth("s16", 10000, length=100)`|46 B|240 B (+421.7 %)|190 B (+313.0 %)|37 B (**-19.6 %**)|
`sawtooth("s16", 100, length=5000)`|1.52 kB|604 B (**-60.2 %**)|347 B (**-77.1 %**)|137 B (**-91.0 %**)|
`sawtooth("s16", 10000, length=5000)`|1.75 kB|11.4 kB (+553.2 %)|9.91 kB (+465.3 %)|102 B (**-94.2 %**)|
`sawtooth("u64", 100, length=100)`|95 B|1.03 kB (+982.1 %)|406 B (+327.4 %)|73 B (**-23.2 %**)|
`sawtooth("u64", 10000, length=100)`|100 B|943 B (+843.0 %)|747 B (+647.0 %)|75 B (**-25.0 %**)|
`sawtooth("u64", 100, length=5000)`|7.81 kB|3.77 kB (**-51.7 %**)|1.6 kB (**-79.5 %**)|327 B (**-95.8 %**)|
`sawtooth("u64", 10000, length=5000)`|4.59 kB|48 kB (+944.3 %)|18 kB (+291.6 %)|322 B (**-93.0 %**)|
`sawtooth("s64", 100, length=100)`|114 B|731 B (+541.2 %)|386 B (+238.6 %)|61 B (**-46.5 %**)|
`sawtooth("s64", 10000, length=100)`|130 B|1.05 kB (+705.4 %)|733 B (+463.8 %)|47 B (**-63.8 %**)|
`sawtooth("s64", 100, length=5000)`|7.83 kB|3.23 kB (**-58.7 %**)|1.57 kB (**-79.9 %**)|314 B (**-96.0 %**)|
`sawtooth("s64", 10000, length=5000)`|3.9 kB|48.6 kB (+1146.1 %)|16.6 kB (+325.3 %)|268 B (**-93.1 %**)|
`sine("u8", 100, length=100)`|42 B|210 B (+400.0 %)|123 B (+192.9 %)|90 B (+114.3 %)|
`sine("u8", 10000, length=100)`|22 B|60 B (+172.7 %)|42 B (+90.9 %)|29 B (+31.8 %)|
`sine("u8", 100, length=5000)`|1.66 kB|375 B (**-77.4 %**)|188 B (**-88.7 %**)|150 B (**-91.0 %**)|
`sine("u8", 10000, length=5000)`|696 B|673 B (-3.3 %)|585 B (**-15.9 %**)|177 B (**-74.6 %**)|
`sine("s16", 100, length=100)`|148 B|293 B (+98.0 %)|223 B (+50.7 %)|211 B (+42.6 %)|
`sine("s16", 10000, length=100)`|42 B|252 B (+500.0 %)|223 B (+431.0 %)|40 B (-4.8 %)|
`sine("s16", 100, length=5000)`|6.92 kB|555 B (**-92.0 %**)|329 B (**-95.2 %**)|305 B (**-95.6 %**)|
`sine("s16", 10000, length=5000)`|1.29 kB|12 kB (+830.8 %)|9.43 kB (+632.1 %)|737 B (**-42.8 %**)|
`sine("u64", 100, length=100)`|1.15 kB|772 B (**-32.7 %**)|568 B (**-50.5 %**)|594 B (**-48.2 %**)|
`sine("u64", 10000, length=100)`|709 B|1.03 kB (+45.0 %)|799 B (+12.7 %)|600 B (**-15.4 %**)|
`sine("u64", 100, length=5000)`|37.3 kB|21.3 kB (**-43.0 %**)|12.6 kB (**-66.1 %**)|8.94 kB (**-76.0 %**)|
`sine("u64", 10000, length=5000)`|28.5 kB|44.5 kB (+56.4 %)|27.8 kB (-2.5 %)|30.4 kB (+6.9 %)|
`sine("s64", 100, length=100)`|1.13 kB|644 B (**-43.2 %**)|579 B (**-48.9 %**)|606 B (**-46.6 %**)|
`sine("s64", 10000, length=100)`|692 B|1.01 kB (+45.5 %)|823 B (+18.9 %)|668 B (-3.5 %)|
`sine("s64", 100, length=5000)`|37.5 kB|18.9 kB (**-49.6 %**)|13.6 kB (**-63.6 %**)|10.7 kB (**-71.5 %**)|
`sine("s64", 10000, length=5000)`|28.4 kB|44.1 kB (+55.2 %)|28.9 kB (+1.6 %)|31.9 kB (+12.2 %)|
`unexpected_jump("u8", 100, length=100)`|75 B|146 B (+94.7 %)|82 B (+9.3 %)|91 B (+21.3 %)|
`unexpected_jump("u8", 10000, length=100)`|69 B|148 B (+114.5 %)|82 B (+18.8 %)|92 B (+33.3 %)|
`unexpected_jump("u8", 100, length=5000)`|3.28 kB|3.52 kB (+7.1 %)|2.63 kB (**-19.9 %**)|2.92 kB (**-11.2 %**)|
`unexpected_jump("u8", 10000, length=5000)`|3.3 kB|3.52 kB (+6.6 %)|2.63 kB (**-20.2 %**)|2.92 kB (**-11.6 %**)|
`unexpected_jump("s16", 100, length=100)`|70 B|151 B (+115.7 %)|113 B (+61.4 %)|123 B (+75.7 %)|
`unexpected_jump("s16", 10000, length=100)`|74 B|146 B (+97.3 %)|109 B (+47.3 %)|130 B (+75.7 %)|
`unexpected_jump("s16", 100, length=5000)`|3.6 kB|3.59 kB (-0.2 %)|3.21 kB (**-10.8 %**)|3.84 kB (+6.6 %)|
`unexpected_jump("s16", 10000, length=5000)`|3.3 kB|3.52 kB (+6.9 %)|3.14 kB (-4.7 %)|3.7 kB (+12.1 %)|
`unexpected_jump("u64", 100, length=100)`|75 B|154 B (+105.3 %)|143 B (+90.7 %)|168 B (+124.0 %)|
`unexpected_jump("u64", 10000, length=100)`|79 B|139 B (+75.9 %)|132 B (+67.1 %)|160 B (+102.5 %)|
`unexpected_jump("u64", 100, length=5000)`|4.44 kB|3.61 kB (**-18.7 %**)|3.79 kB (**-14.5 %**)|4.55 kB (+2.5 %)|
`unexpected_jump("u64", 10000, length=5000)`|3.3 kB|3.51 kB (+6.2 %)|3.72 kB (+12.5 %)|4.4 kB (+33.1 %)|
`unexpected_jump("s64", 100, length=100)`|110 B|164 B (+49.1 %)|144 B (+30.9 %)|175 B (+59.1 %)|
`unexpected_jump("s64", 10000, length=100)`|76 B|151 B (+98.7 %)|139 B (+82.9 %)|163 B (+114.5 %)|
`unexpected_jump("s64", 100, length=5000)`|5.05 kB|3.64 kB (**-27.8 %**)|3.81 kB (**-24.5 %**)|4.63 kB (-8.3 %)|
`unexpected_jump("s64", 10000, length=5000)`|3.33 kB|3.51 kB (+5.5 %)|3.73 kB (+12.0 %)|4.42 kB (+32.5 %)|
|Geometric mean -- CSV datasets||(+133.2 %)|(+106.2 %)|(+14.1 %)|
|Geometric mean -- all datasets||(+83.1 %)|(+26.7 %)|(**-33.9 %**)|

