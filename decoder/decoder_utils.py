from collections.abc import Iterable


class BitReader:
    def __init__(self, chunks: Iterable[bytes]):
        self._it = iter(chunks)
        self._current_chunk = 0
        self._current_chunk_remaining = 0

    def read(self, nbits):
        while nbits > self._current_chunk_remaining:
            self._next_chunk()

        mask = (1 << nbits) - 1
        ret = self._current_chunk & mask
        self._current_chunk = self._current_chunk >> nbits
        self._current_chunk_remaining -= nbits

        return ret

    def read_bit(self):
        return self.read(1)

    def _next_chunk(self):
        b = next(self._it)
        new_chunk = int.from_bytes(b, byteorder="little", signed=False)
        new_chunk_remaining = 8 * len(b)

        self._current_chunk = self._current_chunk | new_chunk << self._current_chunk_remaining
        self._current_chunk_remaining += new_chunk_remaining


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
