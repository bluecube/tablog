import os.path
import subprocess
import json
import sys
import threading
import io


def _find_binary():
    return os.path.join(
        os.path.dirname(__file__), "..", "encoder", "build", "tests", "csv", "csv_tests"
    )


def _serialize_csv_row(row):
    it = iter(row)
    yield next(it).encode("utf-8")
    for s in it:
        yield b","
        yield s.encode("utf-8")
    yield b"\n"


def _serialize_dataset(dataset):
    yield from _serialize_csv_row(dataset.field_names)
    yield from _serialize_csv_row(dataset.field_types)
    for row in dataset:
        yield from _serialize_csv_row(map(str, row))


class EncoderFailed(RuntimeError):
    def __init__(self, command, returncode, stderr):
        self.command = command
        self.returncode = returncode
        self.stderr = stderr

    def __str__(self):
        return f"""Encoder failed with return code {self.returncode!r}
command: {self.command!r}
stderr: """ + "\n".join(map(repr, self.stderr.split(b"\n")))


def encode(dataset, encoder="stream", predictor=None):
    """ Encode a dataset using the CSV encoder and yield blocks of output bytes. """

    command = [_find_binary(), "-", "-", encoder]

    if predictor is not None:
        command.append(predictor)

    def feeder(proc, dataset):
        try:
            for chunk in _serialize_dataset(dataset):
                proc.stdin.write(chunk)
        except BrokenPipeError:
            pass
        finally:
            proc.stdin.close()

    stderr = b""

    def stderr_reader(proc):
        nonlocal stderr
        while True:
            block = proc.stderr.read(io.DEFAULT_BUFFER_SIZE)
            if not block:
                break
            else:
                stderr += block

    proc = subprocess.Popen(
        command,
        bufsize=0,
        stdout=subprocess.PIPE,
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    try:
        stdin_thread = threading.Thread(target=feeder, args=(proc, dataset))
        stdin_thread.start()
        stderr_thread = threading.Thread(target=stderr_reader, args=(proc,))
        stderr_thread.start()

        while True:
            block = proc.stdout.read(io.DEFAULT_BUFFER_SIZE)
            if not block:
                break
            else:
                yield block

    finally:
        if proc.poll() is None:
            proc.terminate()

            try:
                proc.wait(3)
            except subprocess.TimeoutExpired:
                proc.kill()

    if proc.returncode != 0:
        stderr_thread.join(3)
        if stderr_thread.is_alive():
            stderr = None
        raise EncoderFailed(command, proc.returncode, stderr)


def encode_record_stream(dataset, predictor=None):
    remainder = b""
    for block in encode(dataset, encoder="json", predictor=predictor):
        split = block.split(b"\n")
        split[0] = remainder + split[0]
        for line in split[:-1]:
            yield json.loads(line)
        remainder = split[-1]

    if remainder:
        yield json.loads(remainder)


if __name__ == "__main__":
    import datasets

    datasets = datasets.all_datasets()
    dataset = next(datasets)

    encoded = b"".join(encode(next(datasets), "json"))
    print(dataset.name)
    print(encoded.decode("utf-8"))
