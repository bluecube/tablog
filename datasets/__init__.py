from . import csv_datasets
from . import synthetic


def all_datasets(include_synthetic=True):
    yield from csv_datasets.all_datasets()
    if include_synthetic:
        for length in [0, 100, 5000]:
            yield from synthetic.all_datasets(length)


def individual_datasets(include_synthetic=True):
    yield from csv_datasets.individual_datasets()
    if include_synthetic:
        for length in [0, 100, 5000]:
            yield from synthetic.all_datasets(length)
