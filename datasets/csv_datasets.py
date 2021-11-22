import os.path
import re

# Hackish workaround to be able to run this as a script too (rather than just as a part of a package)
try:
    import dataset
except ImportError:
    from . import dataset


class _CsvDataIterCallable:
    def __init__(self, file_path, converters, data_slice):
        self._file_path = file_path
        self._converters = converters
        self._slice = data_slice

    def __call__(self):
        return _CsvDataIterator(self._file_path, self._converters, self._slice)

    def sliced(self, i):
        s = slice(i, i + 1)
        return _CsvDataIterCallable(self._file_path, self._converters, s)


class _CsvDataIterator:
    def __init__(self, file_path, converters, data_slice):
        self._fp = open(file_path)
        next(self._fp)
        next(self._fp)
        self._converters = converters
        self._slice = data_slice

    def __iter__(self):
        return self

    def __next__(self):
        raw_fields = next(self._fp).split(",")
        sliced_converters = self._converters[self._slice]
        sliced_fields = raw_fields[self._slice]
        converted = [converter(x) for converter, x in zip(sliced_converters, sliced_fields)]
        return converted


def _all_csv_files():
    current_dir = os.path.dirname(__file__)
    ret = []
    for (dirpath, dirnames, filenames) in os.walk(current_dir):
        rel_dirpath = os.path.relpath(dirpath, current_dir)
        for filename in filenames:
            if filename.endswith(".csv"):
                file_path = os.path.join(dirpath, filename)
                rel_file_path = os.path.join(rel_dirpath, filename)
                ret.append((rel_file_path, file_path))

    ret.sort()
    return ret


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
    with open(csv_path, "r") as fp:
        field_names = [x.strip() for x in next(fp).split(",")]

        field_types = []
        converters = []
        for t in [x.strip() for x in next(fp).split(",")]:
            converter, converted_t = _parse_type(t)
            field_types.append(converted_t)
            converters.append(converter)

    iter_callable = _CsvDataIterCallable(csv_path, converters, slice(None))

    return dataset.Dataset(
        csv_name,
        field_names, field_types,
        iter_callable,
        None,  # Unknown length
    )


def all_datasets():
    for csv_name, csv_path in _all_csv_files():
        yield _open_dataset(csv_path, csv_name)


def individual_datasets():
    for d in all_datasets():
        for i in range(len(d.field_names)):
            yield dataset.Dataset(
                d.name + "#" + d.field_names[i],
                ["value"],
                [d.field_types[i]],
                d.iter_callable.sliced(i),
                d.length
            )


if __name__ == "__main__":
    print("All datasets")
    dataset.show_content(all_datasets())
    print()
    print("Individual datasets")
    dataset.show_content(individual_datasets())
