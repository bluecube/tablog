from collections.abc import Iterable


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
