class IntType:
    allowed_bitsizes = [8, 16, 32, 64]
    def __init__(self, signed, bitsize):
        if bitsize not in self.allowed_bitsizes:
            raise ValueError("Bit size must be one of 8, 16, 32, 64")

        self.signed = signed
        self.bitsize = bitsize

    def __str__(self):
        return f"{'s' if self.signed else 'u'}{self.bitsize}"

    def __repr__(self):
        return f"{self.__class__.__name__}({self.signed}, {self.bitsize})"

    def __eq__(self, other):
        return self.signed == other.signed and self.bitsize == other.bitsize

    def min(self):
        if self.signed:
            return -(1 << (self.bitsize - 1))
        else:
            return 0

    def max(self):
        if self.signed:
            return (1 << (self.bitsize - 1)) - 1
        else:
            return (1 << self.bitsize) - 1

    def range(self):
        if self.signed:
            tmp = 1 << (self.bitsize - 1)
            return range(-tmp, tmp)
        else:
            return range(0, 1 << self.bitsize)

    def minmax(self):
        r = self.range()
        return (r.start, r.stop - 1)

    def clamp(self, value):
        minimum = self.min()
        if value < minimum:
            return minimum

        maximum = self.max()
        if value > maximum:
            return maximum

        return value

    def bytesize(self):
        return self.bitsize // 8

    @classmethod
    def from_string(cls, s):
        bitsize = int(s[1:])
        if s[0] == "s":
            return cls(True, bitsize)
        elif s[0] == "u":
            return cls(False, bitsize)
        else:
            raise ValueError("First character of type string must be 'u' or 's'")
