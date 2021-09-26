from . import csv_datasets
from . import synthetic

def individual_datasets(include_synthetic=True):
    yield from csv_datasets.individual_datasets()
    if include_synthetic:
        yield from synthetic.all_datasets(length=5000)
