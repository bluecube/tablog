from . import csv_datasets
from . import synthetic

def individual_datasets(synthetic=True):
    yield from csv_datasets.individual_datasets()
    if synthetic:
        yield from synthetic.all(length=5000)
