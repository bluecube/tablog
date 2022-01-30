#!/usr/bin/env python3

"""Generates a markdown table comparing the compression ratio achieved by Tablog
relative to GZiping binary data on all available datasets. """


import math
import gzip
import os.path
import re

from . import encoder_wrappers
import datasets


class MeasurementOnlyFile:
    def __init__(self):
        self.count = 0

    def write(self, b):
        self.count += len(b)


def tablog_compressed_size(dataset):
    return sum(len(chunk) for chunk in encoder_wrappers.csv_encoder(dataset))


def gzip_compressed_size(dataset):
    measurement = MeasurementOnlyFile()
    with gzip.open(measurement, "w") as gz:
        for row in dataset:
            for v, t, in zip(row, dataset.field_types):
                gz.write(v.to_bytes(length=t.bytesize(), byteorder="big", signed=t.signed))

    return measurement.count


def gzip_delta_compressed_size(dataset):
    measurement = MeasurementOnlyFile()
    it = iter(dataset)
    with gzip.open(measurement, "w") as gz:
        try:
            first_row = next(it)
        except StopIteration:
            pass
        else:
            for v, t, in zip(first_row, dataset.field_types):
                gz.write(v.to_bytes(length=t.bytesize(), byteorder="big", signed=t.signed))
            prev_row = first_row

        for row in it:
            for v, prev, t, in zip(row, prev_row, dataset.field_types):
                delta = v - prev
                if delta < 0:
                    delta += 1 << t.bitsize
                gz.write(delta.to_bytes(length=t.bytesize(), byteorder="big", signed=False))
            prev_row = row

    return measurement.count


def table_rows():
    yield "|Dataset|Tablog: Compressed size|Gzip: Compressed size|Gzip Δ: Compressed size|"
    yield "|-------|-----------------------|---------------------|-----------------------|"

    log_ratio_sum = 0
    count = 0

    for dataset in datasets.all_datasets(True):
        try:
            t_size = tablog_compressed_size(dataset)
        except encoder_wrappers.UnsupportedTypeSignature:
            continue

        g_size = gzip_compressed_size(dataset)
        g2_size = gzip_delta_compressed_size(dataset)
        ratio = t_size / g2_size
        log_ratio_sum += math.log(ratio)
        count += 1
        yield f"|`{dataset.name}`|{t_size} B ({format_ratio(ratio)})|{g_size} B|{g2_size} B|"

    mean_ratio = math.exp(log_ratio_sum / count)

    yield f"|Geometric mean|{format_ratio(mean_ratio)}|||"


def format_ratio(r):
    s = f"{100 * r - 100:+.1f} %"
    if r > 1.1:
        return "**" + s + "**"
    else:
        return s


def patched_readme(file):
    patch_start_re = re.compile(r"^\s*#+\s*Compression ratio overview")
    patch_end_re = re.compile(r"^\s*#")

    file_it = iter(file)

    found_start = False
    for line in file_it:
        yield line
        if patch_start_re.match(line):
            found_start = True
            break

    if not found_start:
        raise Exception(f"The expected regex pattern {patch_start_re.pattern!r} was not found in the file")

    yield "\n"
    yield from (row + "\n" for row in table_rows())
    yield "\n"

    for line in file_it:
        if patch_end_re.match(line):
            yield line
            break
    yield from file_it


def patch_readme():
    readme_path = os.path.join(os.path.dirname(__file__), "..", "Readme.md")
    with open(readme_path, "r") as fp:
        lines = list(patched_readme(list(fp)))

    with open(readme_path, "w") as fp:
        for line in lines:
            fp.write(line)


if __name__ == "__main__":
    patch_readme()
