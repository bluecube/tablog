import os.path
import re

def _all_csv():
    current_dir = os.path.dirname(__file__)
    for (dirpath, dirnames, filenames) in os.walk(current_dir):
        rel_dirpath = os.path.relpath(dirpath, current_dir)
        for filename in filenames:
            if filename.endswith(".csv"):
                file_path = os.path.join(dirpath, filename)
                rel_file_path = os.path.join(rel_dirpath, filename)
                yield (file_path, rel_file_path)

def _csv_headers(path):
    with open(path, "r") as fp:
        names = next(fp).split(",")
        types = next(fp).split(",")

    return [(name.strip(), t.strip()) for name, t in zip(names, types)]

""" Retunr tuple of converter function and converted type """
def _parse_type(t):
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

class _CsvColumn:
    def __init__(self, path, column, converter):
        self._fp = open(path, "r")

        next(self._fp)  # ignore title line
        next(self._fp)  # ignore type line

        self._column = column
        self._converter = converter

    def __next__(self):
        return self._converter(next(self._fp).split(",")[self._column].strip())

    def __iter__(self):
        return self


def individual_datasets():
    for path, csv_name in _all_csv():
        columns = _csv_headers(path)
        for i, (column_name, column_type) in enumerate(columns):
            converter, converted_type = _parse_type(column_type)
            yield (csv_name + "#" + column_name, converted_type, _CsvColumn(path, i, converter))

if __name__ == "__main__":
    for name, t, _ in individual_datasets():
        print(f"{name}: {t}")
