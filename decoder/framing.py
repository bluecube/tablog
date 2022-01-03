from collections.abc import Iterable
from typing import Union, Literal
import itertools

block_start_marker = object()
block_end_marker = object()


def decode_framing_raw(data: Union[bytes, Iterable[bytes]]) -> Union[Literal[block_start_marker, block_end_marker], int]:
    """ Convert raw bytes (or iterator of raw byte chunks) to iterable of encoded
    byte values (0-255) and markers (either block_start_marker, or block_end_marker). """
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

