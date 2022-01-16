from __future__ import annotations

from collections.abc import Iterable
from typing import Union, Literal
import itertools

block_start_marker = object()
block_end_marker = object()


def decode_framing_raw(
    data: Union[bytes, Iterable[bytes]]
) -> Union[Literal[block_start_marker, block_end_marker], int]:
    """Convert raw bytes (or iterator of raw byte chunks) to iterable of encoded
    byte values (0-255) and markers (either block_start_marker, or block_end_marker)."""
    escape_byte = b"T"[0]
    start_byte = b"l"[0]
    end_byte = b"#"[0]
    double_escape_byte = b" "[0]

    if isinstance(data, bytes):
        it = iter(data)
    else:
        it = itertools.chain.from_iterable(data)

    had_escape = False

    for b in it:
        if had_escape:
            if b == escape_byte:
                # We didn't use the previous escape byte, output it
                # But keep the current one stored
                yield escape_byte
            else:
                if b == start_byte:
                    yield block_start_marker
                elif b == end_byte:
                    yield block_end_marker
                elif b == double_escape_byte:
                    yield escape_byte
                else:
                    # We didn't use the previous escape byte, output it
                    yield escape_byte
                    # The current byte is not interesting output it as is
                    yield b

                had_escape = False
        elif b == escape_byte:
            had_escape = True
        else:
            yield b

    if had_escape:
        # Output the last escape byte if there was one
        yield escape_byte


def decode_framing(
    data: Union[bytes, Iterable[bytes]]
) -> Iterable[Union[FramingError, FramingBlockIterator]]:
    """Iterates over framing block iterators (see FramingBlockIterator),
    delimited by start and end markers (delimiters are not included in block data).
    Yields FramingError instances when encountering framing errors."""

    raw_framing = decode_framing_raw(data)

    while True:
        junk_size, found = _consume_until_marker(raw_framing, block_start_marker)
        if junk_size != 0:
            yield UnexpectedCharacters(junk_size)
        if not found:
            break

        block = FramingBlockIterator(raw_framing)
        yield block

        frame_error = block._ensure_consumed()

        if frame_error is not None:
            yield frame_error


def _consume_until_marker(it, marker):
    """Consume data from the iterator, until marker is found (compared using
    operator `is`).
    Returns tuple of number of elements consumed before the marker and bool
    indicating whether the marker was found (False means the iterator was
    exhausted."""
    i = -1
    for i, v in enumerate(it):
        if v is marker:
            return (i, True)
    return (i + 1, False)


class FramingBlockIterator:
    _empty_iterator = iter(())

    def __init__(self, it):
        self._it = it

    def __iter__(self):
        return self

    def __next__(self) -> int:
        v = next(self._it)
        if v is block_end_marker:
            self._it = self._empty_iterator
            raise StopIteration()
        return v

    def _ensure_consumed(self):
        """Consume all data up to the following block end marker unless the block
        has already been consumed.
        Returns an error object if there was a problem, None otherwise."""

        if self._it is self._empty_iterator:
            return None

        _, found = _consume_until_marker(self._it, block_end_marker)
        self._it = self._empty_iterator

        if found:
            return None
        else:
            return UnexpectedEndOfData()


class FramingError:
    def __str__(self):
        return "Unspecified framing error"


class UnexpectedCharacters(FramingError):
    def __init__(self, n):
        self.n = n

    def __str__(self):
        return f"Found {self.n} unexpected characters before block start"


class UnexpectedEndOfData(FramingError):
    def __str__(self):
        return "Input data ended unexpectedly"
