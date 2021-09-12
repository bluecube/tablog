import os.path

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
        header = next(fp);

    return [x.strip() for x in header.split(",")]


class _CsvColumn:
    def __init__(self, path, column):
        self._fp = open(path, "r")

        next(self._fp)  # ignore title line
        format_line = next(self._fp).split(",")

        format_field = format_line[column].strip()
        if format_field[0] == "f":
            multiplier = int(format_field[1:])
            self._reader = lambda s: round(float(s) * multiplier)
        else:
            self._reader = int

        self._column = column

    def __next__(self):
        return self._reader(next(self._fp).split(",")[self._column].strip())

    def __iter__(self):
        return self

def individual_datasets():
    for path, title in _all_csv():
        columns = _csv_headers(path)
        for i, name in enumerate(columns):
            yield (title + "#" + name, _CsvColumn(path, i))

if __name__ == "__main__":
    for name, _ in individual_datasets():
        print(name)
