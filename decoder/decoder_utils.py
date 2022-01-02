from collections.abc import Iterable
from typing import Union
import itertools

from . import int_type


class BitReader:
    """ Class that allows reading unaligned bit data from byte stream. """
    def __init__(self, data: Iterable[int]):
        # `bytes` is a valid (and expected/intended) type for `data`
        self._current_chunk = 0
        self._current_chunk_remaining = 0
        self._it = iter(data)

    def read(self, nbits):
        """ Read n bits from the input, return as an int,
        or None if there are no more bits in the input.

        Raises ValueError when there are some bits left in the input,
        but not enough to fulfil the request.  """

        while self._current_chunk_remaining < nbits + 8: # 8 bits lookahead
            if not self._next_byte():
                # Don't try to read more if the input has ended
                break

        if self._current_chunk_remaining < nbits:
            if self._current_chunk_remaining == 0:
                # Clean read when nothing remains
                return None
            else:
                # Not enough bits remain
                raise ValueError(
                    f"Tried to read {nbits} bits with only "
                    f"{self._current_chunk_remaining} bits left in input"
                )

        mask = (1 << nbits) - 1
        ret = self._current_chunk & mask
        self._current_chunk = self._current_chunk >> nbits
        self._current_chunk_remaining -= nbits

        return ret

    def read_bit(self):
        return self.read(1)

    def _next_byte(self):
        try:
            b = next(self._it)
        except StopIteration:
            # Detect the end flag (single 1 bit followed by 0 bytes) and remove
            # from read data
            self._current_chunk_remaining = self._current_chunk.bit_length() - 1
            return False
        else:
            bits = 8
            assert 0 <= b < (1 << bits)
            self._current_chunk = self._current_chunk | b << self._current_chunk_remaining
            self._current_chunk_remaining += bits
            return True


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


def decode_elias_gamma(bit_reader):
    bits = 0
    while bit_reader.read_bit() == 0:
        bits += 1
    return (1 << bits | bit_reader.read(bits)) - 1


class AdaptiveExpGolombDecoder:
    _state_shift = 2

    def __init__(self, bitWidth):
        self._state = (bitWidth // 8) << self._state_shift
        self._max_state = (bitWidth << self._state_shift) - 1

    def decode(self, bit_reader):
        k = self._state >> self._state_shift
        p = decode_elias_gamma(bit_reader)
        self._state = max(0, min(self._state + p - 1, self._max_state))
        return p << k | bit_reader.read(k)


def decode_string(bit_reader):
    return ""


def decode_type(bit_reader):
    signed = bool(bit_reader.read_bit())
    size = 8 << bit_reader.read(2)
    return int_type.IntType(signed, size)
