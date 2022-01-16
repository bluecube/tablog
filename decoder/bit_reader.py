from __future__ import annotations

from collections.abc import Iterable


class IncompleteRead(Exception):
    """Exception notifying the reader that there are no more data available in
    the block of data."""

    def __init__(self, nbits, remaining):
        self.nbits = nbits
        self.remaining = remaining
        super().__init__(
            f"Tried to read {nbits} bits with only {remaining} bits left in input"
        )


class BitReader:
    """Class that allows reading unaligned bit data from byte stream."""

    def __init__(self, data: Iterable[int]):
        # `bytes` is a valid (and expected/intended) type for `data`

        self._remaining = 0
        self._bufer = 0
        self._it = iter(data)
        self._next_byte()
        self._next_byte()

    def read(self, nbits) -> int:
        """Read n bits from the input, return as an int.
        Raises EndOfBlock exception if there are exactly zero bits remaining,
        IncompleteRead if there's not enough (but not zero) bits of data in the input."""

        # Using 9 bits lookahead, so that we always have at least two next bytes
        # loaded and can reliably detect end of block mark
        while self._remaining < nbits + 9:
            if not self._next_byte():
                # Don't try to read more if the input has ended
                break

        if self._remaining < nbits:
            raise IncompleteRead(nbits, self._remaining)

        mask = (1 << nbits) - 1
        ret = self._bufer & mask
        self._bufer = self._bufer >> nbits
        self._remaining -= nbits

        return ret

    def read_bit(self):
        return self.read(1)

    def _next_byte(self):
        try:
            b = next(self._it)
        except StopIteration:
            # Detect the end flag (single 1 bit followed by 0 bytes) and remove
            # from read data
            self._remaining = self._bufer.bit_length() - 1
            return False
        else:
            bits = 8
            assert 0 <= b < (1 << bits)
            self._bufer = self._bufer | b << self._remaining
            self._remaining += bits
            return True

    def end_of_block(self):
        """Returns True if next read would raise IncompleteRead."""
        return self._remaining == 0
