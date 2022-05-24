from . import int_type


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

        if p == 0 and self._state > 0:
            self._state -= 1
        elif p > 1 and self._state < self._max_state:
            self._state += 1

        return p << k | bit_reader.read(k)


def decode_type(bit_reader):
    signed = bool(bit_reader.read_bit())
    size = 8 << bit_reader.read(2)
    return int_type.IntType(signed, size)
