import os.path
import re

# Hackish workaround to be able to run this as a script too (rather than just as a part of a package)
try:
    import dataset
except ImportError:
    from . import dataset

class _CsvDataIterator:
    def __init__(self, fp, converters, data_slice):
        self._fp = fp
        self._converters = converters
        self._slice = data_slice

    def __iter__(self):
        return self

    def __next__(self):
        raw_fields = next(self._fp).split(",")
        sliced = raw_fields[self._slice]
        converted = [converter(x) for converter, x in zip(self._converters, sliced)]
        return converted


def _all_csv_files():
    current_dir = os.path.dirname(__file__)
    for (dirpath, dirnames, filenames) in os.walk(current_dir):
        rel_dirpath = os.path.relpath(dirpath, current_dir)
        for filename in filenames:
            if filename.endswith(".csv"):
                file_path = os.path.join(dirpath, filename)
                rel_file_path = os.path.join(rel_dirpath, filename)
                yield (file_path, rel_file_path)

def _parse_type(t):
    """ Return tuple of converter function and converted type """
    match = re.match(r"(?:f(\d+)\()?([us]\d\d?)\)?", t)
    if not match:
        raise ValueError("Type " + t + " doesn't match the regex")

    if match[1] is None:
        return int, match[2]
    else:
        multiplier = float(match[1])
        def converter(s):
            return round(float(s) * multiplier)
        return converter, match[2]

def _open_dataset(csv_path, csv_name):
    fp = open(csv_path, "r")

    field_names = [x.strip() for x in next(fp).split(",")]

    field_types = []
    converters = []
    for t in [x.strip() for x in next(fp).split(",")]:
        converter, converted_t = _parse_type(t)
        field_types.append(converted_t)
        converters.append(converter)

    data_iterator = _CsvDataIterator(fp, converters, slice(None))

    return dataset.Dataset(csv_name, field_names, field_types, data_iterator)


def all_datasets():
    for csv_path, csv_name in _all_csv_files():
        yield _open_dataset(csv_path, csv_name)

def individual_datasets():
    for d in all_datasets():
        filename = d.data_iterator._fp.name

        for i in range(len(d.field_names)):
            # Duplicate the file pointer
            fp = open(filename)
            # Skip the header lines
            next(fp)
            next(fp)

            yield dataset.Dataset(
                d.name + "#" + d.field_names[i],
                ["value"],
                [d.field_types[i]],
                _CsvDataIterator(
                    fp,
                    [d.data_iterator._converters[i]],
                    slice(i, i + 1)
                )
            )

if __name__ == "__main__":
    print("All datasets")
    dataset.show_content(all_datasets())
    print()
    print("Individual datasets")
    dataset.show_content(individual_datasets())
