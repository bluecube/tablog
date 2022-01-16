""" Wrappers for using the encoder tests from python """


import os
import subprocess
import threading

from . import subprocess_iterator

encoder_tests_dir = os.path.join(os.path.dirname(__file__), "..", "encoder", "build", "tests")


class UnsupportedTypeSignature(Exception):
    pass


def _serialize_csv_row(row):
    it = iter(row)
    yield next(it).encode("utf-8")
    for s in it:
        yield b","
        yield s.encode("utf-8")
    yield b"\n"


def _serialize_dataset(dataset):
    yield from _serialize_csv_row(dataset.field_names)
    yield from _serialize_csv_row(map(str, dataset.field_types))
    for row in dataset:
        yield from _serialize_csv_row(map(str, row))


def csv_encoder(dataset):
    try:
        yield from subprocess_iterator.subprocess_iterator(
            [os.path.join(encoder_tests_dir, "csv", "csv_tests")],
            _serialize_dataset(dataset)
        )
    except subprocess.CalledProcessError as e:
        if e.returncode == 128:
            raise UnsupportedTypeSignature(
                "Tablog CSV driver doesn't support type signature (" +
                ", ".join(str(t) for t in dataset.field_types) +
                ")") from e
        else:
            raise


# Simplify detecting the unsupported type exception when this module is not
# directly imported
csv_encoder.UnsupportedTypeSignature = UnsupportedTypeSignature


class StreamEncoder:
    """ Provides RPC-like interface to the stream_encoder tests, through the call() method """

    def __init__(self, path=None):
        if path is None:
            self._command = [os.path.join(
                encoder_tests_dir,
                "stream_encoder", "stream_encoder_tests"
            )]
        else:
            self._command = [path]

        self._function_call = None
        self._response = None
        self._exception = None
        self._quit_flag = False
        self._subprocess_iterator = None
        self._read_thread = None
        self._cond = threading.Condition()

    def __enter__(self):
        self._start()
        return self

    def __exit__(self, *exc_info):
        if self._is_running():
            self._stop()

    def _start(self):
        self._function_call = None
        self._response = None
        self._exception = None
        self._quit_flag = False
        self._subprocess_iterator = subprocess_iterator.subprocess_iterator(self._command, self._input_iterator())
        self._read_thread = threading.Thread(target=self._output_loop)
        self._read_thread.start()

    def _stop(self):
        with self._cond:
            self._quit_flag = True
            self._cond.notify_all()

        self._read_thread.join()

        if self._exception is not None:
            raise self._exception

    def _is_running(self):
        return self._read_thread.is_alive()

    def call(self, function, *args):
        with self._cond:
            if not self._is_running():
                # Restart in case of a crash
                self._start()

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

                data = data[newline_pos + 1:]

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
