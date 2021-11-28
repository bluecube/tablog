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
