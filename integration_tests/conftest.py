import time
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
stderr: """ + "\n".join(
            map(repr, self.stderr.split(b"\n"))
        )


def _subprocess(command, input_iterator):
    """Run a subprocess, yield its stdout.
    Feeds bytes from input_iterator to the process in a separate thread.
    Ignores stderr if the process returns without error, but includes it in an
    error message if it returns with different status."""

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
    """Make sure that the encoder tests are compiled.
    Return dictionary {test_name: binary_path}."""

    # TODO: Actually run the scons build
    # TODO: Parametrize by different platforms

    tests_path = os.path.join(
        os.path.dirname(__file__), "..", "encoder", "build", "tests"
    )

    paths = {}

    for x in ["unit", "csv", "stream_encoder"]:
        path = os.path.join(tests_path, x, x + "_tests")
        assert os.path.exists(path)
        paths[x] = path

    return paths


@pytest.fixture
def csv_encoder(compiled_encoder):
    """Provides a callable that uses the csv_encoder mechanism to encode a dataset."""

    path = compiled_encoder["csv"]

    def csv_encoder(dataset, encoder="stream", predictor=None):
        command = [path, "-", "-", encoder]

        if predictor is not None:
            command.append(predictor)

        yield from _subprocess(command, _serialize_dataset(dataset))

    return csv_encoder


@pytest.fixture
def csv_encoder_json(csv_encoder):
    """Provides a callable that uses the csv_encoder mechanism to encode a dataset
    using the json format, yielding parsed json object for each output line"""

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


class _StreamEncoder:
    def __init__(self, path):
        self._command = [path]
        self._function_call = None
        self._response = None
        self._exception = None
        self._quit_flag = False
        self._subprocess_iterator = None
        self._read_thread = None
        self._cond = threading.Condition()

    def __enter__(self):
        self._subprocess_iterator = _subprocess(self._command, self._input_iterator())
        self._read_thread = threading.Thread(target=self._output_loop)
        self._read_thread.start()
        return self

    def __exit__(self, *exc_info):
        with self._cond:
            self._quit_flag = True
            self._cond.notify_all()

        self._read_thread.join()

        if self._exception is not None:
            raise self._exception

    def call(self, function, *args):
        with self._cond:
            self._function_call = (
                self._encode(function)
                + b","
                + b",".join(self._encode(arg) for arg in args)
                + b"\n"
            )
            self._cond.notify_all()

            if not self._cond.wait_for(
                lambda: self._response is not None or self._quit_flag, timeout=1
            ):
                raise RuntimeError("stream encoder binary response timed out")

            if self._exception is not None:
                raise self._exception

            if self._response is None:
                raise RuntimeError("Exiting the stream encoder helper during a call!")

            ret = self._response
            self._response = None

        return ret

    def _input_iterator(self):
        with self._cond:
            while not self._quit_flag:
                if self._function_call is not None:
                    yield self._function_call
                    self._function_call = None

                self._cond.wait()

    def _output_loop(self):
        """Reads output from the subprocess, parses it."""
        data = b""
        try:
            while True:  # We stop when the subprocess closes its stdout

                # Keep reading data until we get a newline
                newline_pos = -1
                while newline_pos == -1:
                    block = next(self._subprocess_iterator)
                    data = data + block
                    newline_pos = data.find(b"\n")

                payload_bytes = int(data[:newline_pos].decode("utf-8"))

                data = data[newline_pos + 1 :]

                while len(data) < payload_bytes:
                    block = next(self._subprocess_iterator)
                    data = data + block

                payload = data[:payload_bytes]
                with self._cond:
                    if self._response is not None:
                        raise Exception("Unexpected response " + repr(self._response))
                    self._response = payload
                    self._cond.notify_all()

                data = data[payload_bytes:]
        except StopIteration:
            pass
        except Exception as e:
            with self._cond:
                self._exception = e
                self._quit_flag = True
                self._cond.notify_all()

    @staticmethod
    def _encode(v):
        """Encode an argument for passing to the stream encoder binary"""
        if isinstance(v, bytes):
            return v
        elif isinstance(v, str):
            return v.encode("utf-8")
        else:
            return str(v).encode("utf-8")


@pytest.fixture
def stream_encoder(compiled_encoder):
    with _StreamEncoder(compiled_encoder["stream_encoder"]) as se:
        yield se
