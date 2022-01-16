import subprocess
import threading
import io
import sys


def subprocess_iterator(command, input_iterator):
    """Run a subprocess, yield its stdout.
    Feeds bytes from input_iterator to the process in a separate thread.
    Writes stderr of the process to our own stderr."""

    def feeder(proc, input_iterator):
        try:
            for chunk in input_iterator:
                proc.stdin.write(chunk)
        except BrokenPipeError:
            pass
        finally:
            proc.stdin.close()

    def stderr_reader(proc):
        while True:
            block = proc.stderr.read(io.DEFAULT_BUFFER_SIZE)
            if not block:
                break
            else:
                sys.stderr.write(block.decode("utf-8"))

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
        raise subprocess.CalledProcessError(proc.returncode, command)
