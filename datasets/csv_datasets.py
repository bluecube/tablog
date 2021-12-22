import os.path
import re

from decoder import int_type

# Hackish workaround to be able to run this as a script too (rather than just as a part of a package)
try:
    import dataset
except ImportError:
    from . import dataset


class _CsvDataIterCallable:
    def __init__(self, file_path, converters):
        self._file_path = file_path
        self._converters = converters

    def __call__(self):
        return _CsvDataIterator(self._file_path, self._converters)


class _CsvDataIterator:
    def __init__(self, file_path, converters):
        self._fp = open(file_path)
        next(self._fp)
        next(self._fp)
        self._converters = converters

    def __iter__(self):
        return self

    def __next__(self):
        raw_fields = next(self._fp).split(",")
        return [converter(x) for converter, x in zip(self._converters, raw_fields)]


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
    """Return tuple of converter function and converted type"""

    match = re.match(r"(?:f(\d+)\()?([us]\d\d?)\)?", t)
    if not match:
        raise ValueError("Type " + t + " doesn't match the regex")

    if match[1] is None:
        return int, int_type.IntType.from_string(match[2])
    else:
        multiplier = float(match[1])

        def converter(s):
            return round(float(s) * multiplier)

        return converter, int_type.IntType.from_string(match[2])


def _open_dataset(csv_path, csv_name):
    with open(csv_path, "r") as fp:
        field_names = [x.strip() for x in next(fp).split(",")]

        field_types = []
        converters = []
        for t in [x.strip() for x in next(fp).split(",")]:
            converter, converted_t = _parse_type(t)
            field_types.append(converted_t)
            converters.append(converter)

    iter_callable = _CsvDataIterCallable(csv_path, converters)

    return dataset.Dataset(
        csv_name,
        field_names,
        field_types,
        iter_callable,
        None,  # Unknown length
    )


def all_datasets():
    for csv_name, csv_path in _all_csv_files():
        yield _open_dataset(csv_path, csv_name)


if __name__ == "__main__":
    print("All datasets")
    dataset.show_content(all_datasets())
