#!/usr/bin/env python3

"""Generates a markdown table comparing the compression ratio achieved by Tablog
relative to GZiping binary data on all available datasets. """


import math
import gzip

from . import encoder_wrappers
import datasets


def tablog_compressed_size(dataset):
    return sum(len(chunk) for chunk in encoder_wrappers.csv_encoder(dataset))


def gzip_compressed_size(dataset):
    class MeasurementOnlyFile:
        def __init__(self):
            self.count = 0

        def write(self, b):
            self.count += len(b)

    measurement = MeasurementOnlyFile()
    with gzip.open(measurement, "w") as gz:
        for row in dataset:
            for v, t, in zip(row, dataset.field_types):
                gz.write(v.to_bytes(length=t.bytesize(), byteorder="big", signed=t.signed))

    return measurement.count


print("|Dataset|Tablog: Compressed size|Gzip: Compressed size|")
print("|-------|-----------------------|---------------------|")

log_ratio_sum = 0
count = 0

for dataset in datasets.all_datasets(True):
    try:
        t_size = tablog_compressed_size(dataset)
    except encoder_wrappers.UnsupportedTypeSignature:
        continue

    g_size = gzip_compressed_size(dataset)
    ratio = t_size / g_size
    log_ratio_sum += math.log(ratio)
    count += 1
    print(f"|`{dataset.name}`|{t_size} B ({100 * ratio - 100:+.1f} %)|{g_size} B|")


mean_ratio = math.exp(log_ratio_sum / count)

print(f"|Geometric mean|{100 * mean_ratio - 100:+.1f} %||")
