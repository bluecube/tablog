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
`dataset1/drift.csv`|14.8 kB|49.1 kB (+231.8 %)|45.7 kB (+208.5 %)|12.7 kB (**-14.0 %**)|
`dataset1/power.csv`|449 kB|636 kB (+41.6 %)|540 kB (+20.2 %)|514 kB (+14.4 %)|
`dataset1/soc.csv`|19.3 kB|34.5 kB (+78.9 %)|27.4 kB (+42.4 %)|20 kB (+3.6 %)|
`dataset1/tph.csv`|22 kB|48.6 kB (+121.1 %)|40.9 kB (+86.3 %)|30.8 kB (+40.1 %)|
`gcode/parts.csv`|202 kB|415 kB (+105.4 %)|389 kB (+92.1 %)|241 kB (+19.0 %)|
`phone_imu/imu.csv`|153 kB|271 kB (+76.5 %)|258 kB (+68.0 %)|166 kB (+8.5 %)|
`phone_imu/magnetometer.csv`|50 kB|113 kB (+126.2 %)|99.6 kB (+99.3 %)|48.2 kB (-3.6 %)|
`twinsen/18e755d7ecc2b2a3_data.csv`|321 kB|1.38 MB (+330.5 %)|1.22 MB (+279.9 %)|454 kB (+41.4 %)|
`twinsen/66d8276d3a2480c2_data.csv`|504 kB|1.56 MB (+208.6 %)|1.41 MB (+179.2 %)|659 kB (+30.7 %)|
`empty("u8")`|9 B|33 B (+266.7 %)|26 B (+188.9 %)|26 B (+188.9 %)|
`empty("s16")`|9 B|33 B (+266.7 %)|26 B (+188.9 %)|26 B (+188.9 %)|
`empty("u64")`|9 B|33 B (+266.7 %)|26 B (+188.9 %)|26 B (+188.9 %)|
`empty("s64")`|9 B|33 B (+266.7 %)|26 B (+188.9 %)|26 B (+188.9 %)|
`count_up("u8", length=100)`|22 B|201 B (+813.6 %)|126 B (+472.7 %)|30 B (+36.4 %)|
`count_up("u8", length=5000)`|677 B|680 B (+0.4 %)|340 B (**-49.8 %**)|65 B (**-90.4 %**)|
`count_up("s16", length=100)`|27 B|200 B (+640.7 %)|166 B (+514.8 %)|32 B (+18.5 %)|
`count_up("s16", length=5000)`|639 B|10 kB (+1468.1 %)|8.8 kB (+1276.7 %)|56 B (**-91.2 %**)|
`count_up("u64", length=100)`|24 B|201 B (+737.5 %)|192 B (+700.0 %)|35 B (+45.8 %)|
`count_up("u64", length=5000)`|636 B|11.2 kB (+1660.1 %)|10.4 kB (+1529.7 %)|102 B (**-84.0 %**)|
`count_up("s64", length=100)`|40 B|245 B (+512.5 %)|195 B (+387.5 %)|37 B (-7.5 %)|
`count_up("s64", length=5000)`|653 B|12.6 kB (+1836.6 %)|10.3 kB (+1482.2 %)|104 B (**-84.1 %**)|
`minor7chord("u8", 100, length=100)`|41 B|207 B (+404.9 %)|125 B (+204.9 %)|97 B (+136.6 %)|
`minor7chord("u8", 10000, length=100)`|26 B|69 B (+165.4 %)|53 B (+103.8 %)|32 B (+23.1 %)|
`minor7chord("u8", 100, length=5000)`|1.31 kB|1.52 kB (+16.1 %)|1.03 kB (**-21.2 %**)|489 B (**-62.7 %**)|
`minor7chord("u8", 10000, length=5000)`|714 B|688 B (-3.6 %)|740 B (+3.6 %)|214 B (**-70.0 %**)|
`minor7chord("s16", 100, length=100)`|145 B|331 B (+128.3 %)|229 B (+57.9 %)|229 B (+57.9 %)|
`minor7chord("s16", 10000, length=100)`|38 B|286 B (+652.6 %)|229 B (+502.6 %)|41 B (+7.9 %)|
`minor7chord("s16", 100, length=5000)`|6.26 kB|3.19 kB (**-49.0 %**)|2.18 kB (**-65.1 %**)|1.86 kB (**-70.2 %**)|
`minor7chord("s16", 10000, length=5000)`|1.27 kB|13.1 kB (+935.5 %)|9.84 kB (+676.4 %)|954 B (**-24.7 %**)|
`minor7chord("u64", 100, length=100)`|1.14 kB|1.08 kB (-5.1 %)|820 B (**-28.3 %**)|813 B (**-28.9 %**)|
`minor7chord("u64", 10000, length=100)`|751 B|1.04 kB (+39.0 %)|817 B (+8.8 %)|630 B (**-16.1 %**)|
`minor7chord("u64", 100, length=5000)`|36.8 kB|33.1 kB (**-10.0 %**)|21.2 kB (**-42.4 %**)|19 kB (**-48.2 %**)|
`minor7chord("u64", 10000, length=5000)`|29 kB|48.8 kB (+68.4 %)|36.8 kB (+27.0 %)|31.3 kB (+7.8 %)|
`minor7chord("s64", 100, length=100)`|1.13 kB|1.07 kB (-5.0 %)|829 B (**-26.6 %**)|824 B (**-27.1 %**)|
`minor7chord("s64", 10000, length=100)`|734 B|1.02 kB (+39.0 %)|829 B (+12.9 %)|692 B (-5.7 %)|
`minor7chord("s64", 100, length=5000)`|36.8 kB|30.5 kB (**-17.0 %**)|22 kB (**-40.2 %**)|20 kB (**-45.5 %**)|
`minor7chord("s64", 10000, length=5000)`|29 kB|48.6 kB (+67.8 %)|37.4 kB (+29.1 %)|32.7 kB (+12.9 %)|
`random("u8", length=100)`|142 B|224 B (+57.7 %)|129 B (-9.2 %)|129 B (-9.2 %)|
`random("u8", length=5000)`|6.41 kB|7.42 kB (+15.8 %)|5.03 kB (**-21.5 %**)|5.03 kB (**-21.5 %**)|
`random("s16", length=100)`|264 B|369 B (+39.8 %)|229 B (**-13.3 %**)|229 B (**-13.3 %**)|
`random("s16", length=5000)`|11.5 kB|14.6 kB (+27.8 %)|10 kB (**-12.5 %**)|10 kB (**-12.5 %**)|
`random("u64", length=100)`|1.34 kB|1.11 kB (**-17.1 %**)|829 B (**-38.1 %**)|829 B (**-38.1 %**)|
`random("u64", length=5000)`|42.1 kB|51.3 kB (+21.8 %)|40 kB (-5.0 %)|40 kB (-5.0 %)|
`random("s64", length=100)`|1.33 kB|1.12 kB (**-15.6 %**)|829 B (**-37.7 %**)|829 B (**-37.7 %**)|
`random("s64", length=5000)`|42.1 kB|51.4 kB (+22.1 %)|40 kB (-4.9 %)|40 kB (-4.9 %)|
`random_smooth("u8", 100, length=100)`|34 B|167 B (+391.2 %)|101 B (+197.1 %)|68 B (+100.0 %)|
`random_smooth("u8", 10000, length=100)`|23 B|40 B (+73.9 %)|29 B (+26.1 %)|30 B (+30.4 %)|
`random_smooth("u8", 100, length=5000)`|1.15 kB|3.62 kB (+216.2 %)|3.03 kB (+164.5 %)|909 B (**-20.6 %**)|
`random_smooth("u8", 10000, length=5000)`|636 B|92 B (**-85.5 %**)|48 B (**-92.5 %**)|49 B (**-92.3 %**)|
`random_smooth("s16", 100, length=100)`|98 B|332 B (+238.8 %)|229 B (+133.7 %)|193 B (+96.9 %)|
`random_smooth("s16", 10000, length=100)`|28 B|74 B (+164.3 %)|58 B (+107.1 %)|47 B (+67.9 %)|
`random_smooth("s16", 100, length=5000)`|3.8 kB|14 kB (+269.9 %)|10 kB (+164.0 %)|6.72 kB (+76.9 %)|
`random_smooth("s16", 10000, length=5000)`|1.23 kB|12 kB (+879.6 %)|9.12 kB (+641.7 %)|476 B (**-61.3 %**)|
`random_smooth("u64", 100, length=100)`|968 B|1.05 kB (+8.8 %)|813 B (**-16.0 %**)|658 B (**-32.0 %**)|
`random_smooth("u64", 10000, length=100)`|778 B|949 B (+22.0 %)|704 B (-9.5 %)|668 B (**-14.1 %**)|
`random_smooth("u64", 100, length=5000)`|34 kB|49.4 kB (+45.4 %)|37.6 kB (+10.6 %)|24.4 kB (**-28.2 %**)|
`random_smooth("u64", 10000, length=5000)`|27.2 kB|48 kB (+76.8 %)|35.7 kB (+31.5 %)|23.7 kB (**-12.9 %**)|
`random_smooth("s64", 100, length=100)`|1 kB|1.05 kB (+4.9 %)|829 B (**-17.5 %**)|636 B (**-36.7 %**)|
`random_smooth("s64", 10000, length=100)`|728 B|898 B (+23.4 %)|693 B (-4.8 %)|614 B (**-15.7 %**)|
`random_smooth("s64", 100, length=5000)`|34.2 kB|49.5 kB (+44.8 %)|37.9 kB (+10.8 %)|25.4 kB (**-25.7 %**)|
`random_smooth("s64", 10000, length=5000)`|25.3 kB|46.1 kB (+82.1 %)|35.7 kB (+41.0 %)|26.4 kB (+4.3 %)|
`random_step("u8", 100, length=100)`|24 B|42 B (+75.0 %)|31 B (+29.2 %)|33 B (+37.5 %)|
`random_step("u8", 10000, length=100)`|23 B|40 B (+73.9 %)|29 B (+26.1 %)|30 B (+30.4 %)|
`random_step("u8", 100, length=5000)`|699 B|252 B (**-63.9 %**)|182 B (**-74.0 %**)|213 B (**-69.5 %**)|
`random_step("u8", 10000, length=5000)`|636 B|92 B (**-85.5 %**)|48 B (**-92.5 %**)|49 B (**-92.3 %**)|
`random_step("s16", 100, length=100)`|29 B|51 B (+75.9 %)|34 B (+17.2 %)|35 B (+20.7 %)|
`random_step("s16", 10000, length=100)`|24 B|43 B (+79.2 %)|30 B (+25.0 %)|31 B (+29.2 %)|
`random_step("s16", 100, length=5000)`|770 B|348 B (**-54.8 %**)|235 B (**-69.5 %**)|280 B (**-63.6 %**)|
`random_step("s16", 10000, length=5000)`|637 B|100 B (**-84.3 %**)|56 B (**-91.2 %**)|56 B (**-91.2 %**)|
`random_step("u64", 100, length=100)`|37 B|72 B (+94.6 %)|41 B (+10.8 %)|42 B (+13.5 %)|
`random_step("u64", 10000, length=100)`|37 B|72 B (+94.6 %)|41 B (+10.8 %)|42 B (+13.5 %)|
`random_step("u64", 100, length=5000)`|1.35 kB|996 B (**-26.3 %**)|681 B (**-49.6 %**)|718 B (**-46.9 %**)|
`random_step("u64", 10000, length=5000)`|649 B|336 B (**-48.2 %**)|114 B (**-82.4 %**)|94 B (**-85.5 %**)|
`random_step("s64", 100, length=100)`|36 B|67 B (+86.1 %)|41 B (+13.9 %)|42 B (+16.7 %)|
`random_step("s64", 10000, length=100)`|36 B|70 B (+94.4 %)|41 B (+13.9 %)|42 B (+16.7 %)|
`random_step("s64", 100, length=5000)`|1.29 kB|934 B (**-27.7 %**)|631 B (**-51.1 %**)|664 B (**-48.6 %**)|
`random_step("s64", 10000, length=5000)`|649 B|322 B (**-50.4 %**)|114 B (**-82.4 %**)|94 B (**-85.5 %**)|
`sawtooth("u8", 100, length=100)`|45 B|215 B (+377.8 %)|129 B (+186.7 %)|38 B (**-15.6 %**)|
`sawtooth("u8", 10000, length=100)`|22 B|45 B (+104.5 %)|34 B (+54.5 %)|36 B (+63.6 %)|
`sawtooth("u8", 100, length=5000)`|1.86 kB|389 B (**-79.1 %**)|211 B (**-88.6 %**)|119 B (**-93.6 %**)|
`sawtooth("u8", 10000, length=5000)`|666 B|289 B (**-56.6 %**)|264 B (**-60.4 %**)|83 B (**-87.5 %**)|
`sawtooth("s16", 100, length=100)`|51 B|345 B (+576.5 %)|229 B (+349.0 %)|44 B (**-13.7 %**)|
`sawtooth("s16", 10000, length=100)`|50 B|240 B (+380.0 %)|200 B (+300.0 %)|43 B (**-14.0 %**)|
`sawtooth("s16", 100, length=5000)`|1.52 kB|604 B (**-60.3 %**)|352 B (**-76.9 %**)|146 B (**-90.4 %**)|
`sawtooth("s16", 10000, length=5000)`|1.76 kB|11.4 kB (+551.7 %)|9.92 kB (+464.4 %)|109 B (**-93.8 %**)|
`sawtooth("u64", 100, length=100)`|99 B|1.03 kB (+938.4 %)|413 B (+317.2 %)|77 B (**-22.2 %**)|
`sawtooth("u64", 10000, length=100)`|104 B|943 B (+806.7 %)|751 B (+622.1 %)|79 B (**-24.0 %**)|
`sawtooth("u64", 100, length=5000)`|7.82 kB|3.77 kB (**-51.8 %**)|1.61 kB (**-79.4 %**)|334 B (**-95.7 %**)|
`sawtooth("u64", 10000, length=5000)`|4.6 kB|48 kB (+943.4 %)|18 kB (+291.4 %)|331 B (**-92.8 %**)|
`sawtooth("s64", 100, length=100)`|118 B|731 B (+519.5 %)|396 B (+235.6 %)|67 B (**-43.2 %**)|
`sawtooth("s64", 10000, length=100)`|134 B|1.05 kB (+681.3 %)|739 B (+451.5 %)|53 B (**-60.4 %**)|
`sawtooth("s64", 100, length=5000)`|7.84 kB|3.23 kB (**-58.7 %**)|1.58 kB (**-79.8 %**)|324 B (**-95.9 %**)|
`sawtooth("s64", 10000, length=5000)`|3.91 kB|48.6 kB (+1144.9 %)|16.6 kB (+325.0 %)|276 B (**-92.9 %**)|
`sine("u8", 100, length=100)`|46 B|210 B (+356.5 %)|129 B (+180.4 %)|101 B (+119.6 %)|
`sine("u8", 10000, length=100)`|25 B|60 B (+140.0 %)|47 B (+88.0 %)|35 B (+40.0 %)|
`sine("u8", 100, length=5000)`|1.66 kB|375 B (**-77.5 %**)|193 B (**-88.4 %**)|163 B (**-90.2 %**)|
`sine("u8", 10000, length=5000)`|700 B|673 B (-3.9 %)|592 B (**-15.4 %**)|187 B (**-73.3 %**)|
`sine("s16", 100, length=100)`|151 B|293 B (+94.0 %)|229 B (+51.7 %)|218 B (+44.4 %)|
`sine("s16", 10000, length=100)`|46 B|252 B (+447.8 %)|229 B (+397.8 %)|45 B (-2.2 %)|
`sine("s16", 100, length=5000)`|6.93 kB|555 B (**-92.0 %**)|335 B (**-95.2 %**)|314 B (**-95.5 %**)|
`sine("s16", 10000, length=5000)`|1.29 kB|12 kB (+827.9 %)|9.43 kB (+630.2 %)|749 B (**-42.0 %**)|
`sine("u64", 100, length=100)`|1.15 kB|772 B (**-32.9 %**)|574 B (**-50.1 %**)|600 B (**-47.9 %**)|
`sine("u64", 10000, length=100)`|713 B|1.03 kB (+44.2 %)|804 B (+12.8 %)|606 B (**-15.0 %**)|
`sine("u64", 100, length=5000)`|37.3 kB|21.3 kB (**-43.0 %**)|12.6 kB (**-66.1 %**)|8.95 kB (**-76.0 %**)|
`sine("u64", 10000, length=5000)`|28.5 kB|44.5 kB (+56.4 %)|27.8 kB (-2.5 %)|30.4 kB (+6.9 %)|
`sine("s64", 100, length=100)`|1.14 kB|644 B (**-43.4 %**)|585 B (**-48.6 %**)|611 B (**-46.3 %**)|
`sine("s64", 10000, length=100)`|696 B|1.01 kB (+44.7 %)|829 B (+19.1 %)|673 B (-3.3 %)|
`sine("s64", 100, length=5000)`|37.5 kB|18.9 kB (**-49.6 %**)|13.6 kB (**-63.6 %**)|10.7 kB (**-71.5 %**)|
`sine("s64", 10000, length=5000)`|28.4 kB|44.1 kB (+55.1 %)|28.9 kB (+1.6 %)|31.9 kB (+12.2 %)|
`unexpected_jump("u8", 100, length=100)`|79 B|146 B (+84.8 %)|93 B (+17.7 %)|102 B (+29.1 %)|
`unexpected_jump("u8", 10000, length=100)`|73 B|148 B (+102.7 %)|93 B (+27.4 %)|101 B (+38.4 %)|
`unexpected_jump("u8", 100, length=5000)`|3.29 kB|3.52 kB (+7.0 %)|2.64 kB (**-19.6 %**)|2.93 kB (**-10.9 %**)|
`unexpected_jump("u8", 10000, length=5000)`|3.3 kB|3.52 kB (+6.4 %)|2.65 kB (**-19.9 %**)|2.93 kB (**-11.3 %**)|
`unexpected_jump("s16", 100, length=100)`|74 B|151 B (+104.1 %)|124 B (+67.6 %)|133 B (+79.7 %)|
`unexpected_jump("s16", 10000, length=100)`|78 B|146 B (+87.2 %)|117 B (+50.0 %)|140 B (+79.5 %)|
`unexpected_jump("s16", 100, length=5000)`|3.61 kB|3.59 kB (-0.3 %)|3.23 kB (**-10.5 %**)|3.86 kB (+6.9 %)|
`unexpected_jump("s16", 10000, length=5000)`|3.3 kB|3.52 kB (+6.7 %)|3.16 kB (-4.5 %)|3.71 kB (+12.4 %)|
`unexpected_jump("u64", 100, length=100)`|79 B|154 B (+94.9 %)|147 B (+86.1 %)|175 B (+121.5 %)|
`unexpected_jump("u64", 10000, length=100)`|83 B|139 B (+67.5 %)|137 B (+65.1 %)|167 B (+101.2 %)|
`unexpected_jump("u64", 100, length=5000)`|4.44 kB|3.61 kB (**-18.8 %**)|3.8 kB (**-14.5 %**)|4.56 kB (+2.7 %)|
`unexpected_jump("u64", 10000, length=5000)`|3.31 kB|3.51 kB (+6.1 %)|3.72 kB (+12.5 %)|4.41 kB (+33.2 %)|
`unexpected_jump("s64", 100, length=100)`|114 B|164 B (+43.9 %)|148 B (+29.8 %)|183 B (+60.5 %)|
`unexpected_jump("s64", 10000, length=100)`|80 B|151 B (+88.8 %)|147 B (+83.8 %)|170 B (+112.5 %)|
`unexpected_jump("s64", 100, length=5000)`|5.05 kB|3.64 kB (**-27.9 %**)|3.82 kB (**-24.4 %**)|4.64 kB (-8.2 %)|
`unexpected_jump("s64", 10000, length=5000)`|3.34 kB|3.51 kB (+5.3 %)|3.74 kB (+12.1 %)|4.43 kB (+32.6 %)|
|Geometric mean -- CSV datasets||(+133.0 %)|(+106.1 %)|(+14.1 %)|
|Geometric mean -- all datasets||(+73.8 %)|(+25.4 %)|(**-33.0 %**)|

