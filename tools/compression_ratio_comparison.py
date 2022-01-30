#!/usr/bin/env python3

"""Generates a markdown table comparing the compression ratio achieved by Tablog
relative to GZiping binary data on all available datasets. """


import math
import gzip
import os.path
import re
import operator

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
    with gzip.open(measurement, "w", compresslevel=9) as gz:
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


def generate_table_data_rows(datasets, size_functions):
    assert len(size_functions) > 1
    log_ratio_sums = [0] * len(size_functions)
    count = 0
    csv_log_ratio_sums = [0] * len(size_functions)
    csv_count = 0
    for dataset in datasets:
        try:
            sizes = [fun(dataset) for fun in size_functions]
        except encoder_wrappers.UnsupportedTypeSignature:
            continue

        log_ratios = [math.log(size / sizes[0]) for size in sizes]
        log_ratio_sums = list(map(operator.add, log_ratio_sums, log_ratios))
        count += 1

        if dataset.name.endswith(".csv"):
            csv_log_ratio_sums = list(map(operator.add, csv_log_ratio_sums, log_ratios))
            csv_count += 1

        yield f"`{dataset.name}`|{sizes[0]} B|" + \
            "|".join(f"{size} B ({format_ratio(size / sizes[0])})" for size in sizes[1:]) + \
            "|"

    yield "|Geometric mean -- CSV datasets||" + \
        "|".join(f"({format_ratio(math.exp(csv_log_ratio_sum / csv_count))})" for csv_log_ratio_sum in csv_log_ratio_sums[1:]) + \
        "|"
    yield "|Geometric mean -- all datasets||" + \
        "|".join(f"({format_ratio(math.exp(log_ratio_sum / count))})" for log_ratio_sum in log_ratio_sums[1:]) + \
        "|"


def generate_table_rows():
    yield "|Dataset|Tablog: Compressed size|Gzip: Compressed size|Gzip Δ: Compressed size|"
    yield "|-------|-----------------------|---------------------|-----------------------|"

    yield from generate_table_data_rows(
        datasets.all_datasets(True),
        [tablog_compressed_size, gzip_compressed_size, gzip_delta_compressed_size]
    )


def format_ratio(r):
    s = f"{100 * r - 100:+.1f} %"
    if r < 0.9:
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
    yield from (row + "\n" for row in generate_table_rows())
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
