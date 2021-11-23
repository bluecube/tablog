from . import dataset
from . import csv_datasets
from . import synthetic


def all_datasets(include_synthetic=True):
    """Yield all csv and synthetic datasets available"""
    yield from csv_datasets.all_datasets()
    if include_synthetic:
        for length in [0, 100, 5000]:
            yield from synthetic.all_datasets(length)


def individual_datasets(*args, **kwargs):
    """Yield all csv and synthetic datasets available, separated so that each
    dataset has exactly one column"""

    # Causes each dataset to be completely loaded to memory before being split
    # into individual column datasets. This speeds up the processing, but
    # potentially might take a lot of memory.
    memoize = True

    for ds in all_datasets(*args, **kwargs):
        if len(ds.field_names) == 1:
            yield ds
        elif memoize:
            data = list(ds.iter_callable())

            for i in range(len(ds.field_names)):
                yield dataset.Dataset(
                    ds.name + "#" + ds.field_names[i],
                    ["value"],
                    [ds.field_types[i]],
                    lambda i=i, data=data: ([fields[i]] for fields in data),
                    len(data),
                )
        else:
            for i in range(len(ds.field_names)):
                yield dataset.Dataset(
                    ds.name + "#" + ds.field_names[i],
                    ["value"],
                    [ds.field_types[i]],
                    lambda i=i, ds=ds: ([fields[i]] for fields in ds.iter_callable()),
                    ds.length,
                )
