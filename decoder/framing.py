from collections.abc import Iterable
from typing import Union
import itertools


class FramingDecoder:
    block_start_marker = object()
    block_end_marker = object()

    _escape_byte = b"T"[0]
    _start_byte = b"l"[0]
    _end_byte = b"#"[0]
    _double_escape_byte = b" "[0]

    def __init__(self, data: Union[bytes, Iterable[bytes]]):
        self._had_escape = False

        if isinstance(data, bytes):
            self._it = iter(data)
        else:
            self._it = itertools.chain.from_iterable(data)

    def raw_iterator(self):
        """Iterates over individual encoded bytes and marker objects """
        for b in self._it:
            if self._had_escape:
                if b == self._escape_byte:
                    # We didn't use the previous escape byte, output it
                    # But keep the current one stored
                    yield self._escape_byte
                else:
                    if b == self._start_byte:
                        yield self.block_start_marker
                    elif b == self._end_byte:
                        yield self.block_end_marker
                    elif b == self._double_escape_byte:
                        yield self._escape_byte
                    else:
                        # We didn't use the previous escape byte, output it
                        yield self._escape_byte
                        # The current byte is not interesting output it as is
                        yield b

                    self._had_escape = False
            elif b == self._escape_byte:
                self._had_escape = True
            else:
                yield b

        if self._had_escape:
            # Output the last escape byte if there was one
            yield self._escape_byte
