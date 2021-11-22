import os.path
import subprocess
import json
import threading
import io
import pytest


class SubprocessFailed(RuntimeError):
    def __init__(self, command, returncode, stderr):
        self.command = command
        self.returncode = returncode
        self.stderr = stderr

    def __str__(self):
        return f"""Encoder failed with return code {self.returncode!r}
command: {self.command!r}
stderr: """ + "\n".join(map(repr, self.stderr.split(b"\n")))


def _subprocess(command, input_iterator):
    """ Run a subprocess, yield its stdout.
    Feeds bytes from input_iterator to the process in a separate thread.
    Ignores stderr if the process returns without error, but includes it in an
    error message if it returns with different status. """

    def feeder(proc, input_iterator):
        try:
            for chunk in input_iterator:
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
        stdin_thread = threading.Thread(target=feeder, args=(proc, input_iterator))
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
        raise SubprocessFailed(command, proc.returncode, stderr)


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


@pytest.fixture(scope="session")  # Minimize the attempts to recompile the encoder
def compiled_encoder():
    """ Make sure that the encoder tests are compiled.
    Return dictionary {test_name: binary_path}. """

    # TODO: Actually run the scons build
    # TODO: Parametrize by different platforms

    tests_path = os.path.join(
        os.path.dirname(__file__), "..", "encoder", "build", "tests"
    )

    paths = {}

    for x in ["unit", "csv", "stream_encoder"]:
        path = os.path.join(tests_path, x, x + "_tests")
        assert(os.path.exists(path))
        paths[x] = path

    return paths


@pytest.fixture
def csv_encoder(compiled_encoder):
    """ Provides a callable that uses the csv_encoder mechanism to encode a dataset. """

    path = compiled_encoder["csv"]

    def csv_encoder(dataset, encoder="stream", predictor=None):
        command = [path, "-", "-", encoder]

        if predictor is not None:
            command.append(predictor)

        yield from _subprocess(command, _serialize_dataset(dataset))

    return csv_encoder


@pytest.fixture
def csv_encoder_json(csv_encoder):
    """ Provides a callable that uses the csv_encoder mechanism to encode a dataset
    using the json format, yielding parsed json object for each output line """

    def csv_encoder_json(dataset, predictor=None):
        unprocessed = b""
        for block in csv_encoder(dataset, encoder="json", predictor=predictor):
            split = block.split(b"\n")
            split[0] = unprocessed + split[0]
            for line in split[:-1]:
                yield json.loads(line)
            unprocessed = split[-1]

        if unprocessed:
            yield json.loads(unprocessed)

    return csv_encoder_json
