import os.path
import subprocess


def _find_binary():
    return os.path.join(
        os.path.dirname(__file__), "..", "encoder", "build", "tests", "csv", "csv_tests"
    )


def _serialize_json_row(row):
    it = iter(row)
    yield next(it).encode("utf-8")
    for s in it:
        yield b","
        yield s.encode("utf-8")
    yield b"\n"


def _serialize_dataset(dataset):
    yield from _serialize_json_row(dataset.field_names)
    yield from _serialize_json_row(dataset.field_types)
    for row in dataset.data_iterator:
        yield from _serialize_json_row(map(str, row))


def encode(dataset, encoder="stream", predictor="linear3"):
    stdin = b"".join(_serialize_dataset(dataset))

    process = subprocess.run(
        [_find_binary(), "-", "-", encoder, predictor],
        input=stdin,
        check=True,
        capture_output=True,
    )

    return process.stdout


if __name__ == "__main__":
    import datasets

    datasets = datasets.all_datasets()
    dataset = next(datasets)

    encoded = encode(next(datasets), "stat")
    print(dataset.name)
    print(encoded.decode("utf-8"))
