from collections.abc import Iterable
from typing import Union


class BitReader:
    def __init__(self, data: Union[bytes, Iterable[bytes]]):
        self._current_chunk = 0
        self._current_chunk_remaining = 0

        if isinstance(data, bytes):
            self._add_chunk(data)
            self._it = iter([])
        else:
            self._it = iter(data)

    def read(self, nbits):
        """ Read n bits from the input, return as an int,
        or None if there are no more bits in the input.

        Raises ValueError when there are some bits left in the input,
        but not enough to fulfil the request.  """
        try:
            while nbits > self._current_chunk_remaining:
                self._next_chunk()
        except StopIteration as e:
            if self._current_chunk_remaining > 0:
                raise ValueError(
                    f"Tried to read {nbits} bits with only "
                    f"{self._current_chunk_remaining} bits left in input"
                ) from e
            else:
                return None

        mask = (1 << nbits) - 1
        ret = self._current_chunk & mask
        self._current_chunk = self._current_chunk >> nbits
        self._current_chunk_remaining -= nbits

        return ret

    def read_bit(self):
        return self.read(1)

    def _add_chunk(self, b):
        new_chunk = int.from_bytes(b, byteorder="little", signed=False)
        new_chunk_remaining = 8 * len(b)

        self._current_chunk = self._current_chunk | new_chunk << self._current_chunk_remaining
        self._current_chunk_remaining += new_chunk_remaining

    def _next_chunk(self):
        self._add_chunk(next(self._it))


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
