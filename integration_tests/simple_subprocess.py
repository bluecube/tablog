import subprocess as sys_subprocess
import threading
import io
import sys


class SubprocessFailed(RuntimeError):
    def __init__(self, command, returncode):
        self.command = command
        self.returncode = returncode

    def __str__(self):
        return (
            f"Encoder failed with return code {self.returncode!r}"
            f" (command: {self.command!r})"
        )


def subprocess(command, input_iterator):
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

    proc = sys_subprocess.Popen(
        command,
        bufsize=0,
        stdout=sys_subprocess.PIPE,
        stdin=sys_subprocess.PIPE,
        stderr=sys_subprocess.PIPE,
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
            except sys_subprocess.TimeoutExpired:
                proc.kill()

    if proc.returncode != 0:
        stderr_thread.join(3)
        raise SubprocessFailed(command, proc.returncode)
