from . import csv_datasets
from . import synthetic

def individual_datasets():
    yield from csv_datasets.individual_datasets()
    yield from synthetic.all(length=5000)
