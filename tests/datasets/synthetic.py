import math
import itertools
import numpy.random
import inspect
import sys
import hashlib

def all(length):
    dataset_name_suffix = f", length={length})"
    currentmod = sys.modules[__name__]

    for func_name, func in inspect.getmembers(currentmod, inspect.isfunction):
        if func_name.startswith("_"):
            continue
        if func is all:
            continue
        for t in ["s8", "u16", "u32", "s64"]:  # Not completely random selection
            dataset_name_prefix = f'{func_name}("{t}"'
            if "period" in inspect.signature(func).parameters:
                for period in [100, 10000]:
                    yield (
                        dataset_name_prefix + f', period={period}' + dataset_name_suffix,
                        func(t, period, length)
                    )
            else:
                yield (
                    dataset_name_prefix + dataset_name_suffix,
                    func(t, length)
                )

def _parse_type(s):
    """ Return semi-open interval of output vaues """
    scale_bits = int(s[1:])
    if s[0] == "s":
        high = 1 << (scale_bits - 1)
        low = -high
    elif s[0] == "u":
        low = 0
        high = 1 << scale_bits
    else:
        raise ValueError("Scale must be 's' or 'u'")

    return (low, high)

def _remap(f, input_min, input_max, x_scale, t, length):
    """ Turn a function giving a value from <input_min, input_max> to an infinite iterator of
    integers between <0, 2**scale_bits> (signed == "u") or
    <-2**(scale_bits - 1), 2**(scale_bits - 1) - 1> (signed == "s") """
    low, high = _parse_type(t)

    y_scale = (high - low - 1) / (input_max - input_min)
    y_offset = input_min * y_scale + low

    return (int(f(i * x_scale) * y_scale + y_offset) for i in range(length))

def sine(t, period, length):
    return _remap(math.sin, -1, 1, 2 * math.pi / period, t, length)

def sawtooth(t, period, length):
    return _remap(lambda x: math.modf(x)[0], 0, 1, 1/period, t, length)

def _random(gen, t, length):
    low, high = _parse_type(t)
    return (gen.integers(low, high) for _ in range(length))

def _make_generator(*args):
    hasher = hashlib.sha1()
    hasher.update(repr(args).encode())
    seed = int.from_bytes(hasher.digest(), byteorder='big')
    return numpy.random.Generator(numpy.random.PCG64(seed))

def random_piecewise_constant(t, period, length):
    gen = _make_generator(t, period)
    remaining = length
    for value in _random(gen, t, length):
        segment_count = min(period // 2 + gen.geometric(2 / period), remaining)
        remaining = remaining - segment_count

        yield from itertools.repeat(value, segment_count)

        if remaining == 0:
            break;

def random_smooth(t, period, length):
    gen = _make_generator(t, period)
    remaining = length
    it = _random(gen, t, length + 4)

    prev = next(it)
    while remaining:
        current = next(it)
        segment_count_raw = period // 2 + gen.geometric(2 / period)
        segment_count = min(segment_count_raw, remaining)
        remaining = remaining - segment_count

        # t = (1 - math.cos(i * math.pi / segment_count_raw)) / 2
        # v = prev + t * (current - prev)

        a = math.pi / segment_count_raw
        b = 0.5 * (prev + current)
        c = 0.5 * (current - prev)

        yield from (b - math.cos(i * a) * c for i in range(segment_count))

        prev = current

def random(t, length):
    return _random(_make_generator(t), t, length)

def count_up(t, length):
    low, high = _parse_type(t)
    scale = high - low
    return ((i % scale) + low for i in range(length))

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    import numpy

    length = 2000000
    it = random_piecewise_constant("s8", 50000, length)

    a = numpy.fromiter(it, dtype=numpy.int64, count=length)

    plt.plot(a)
    plt.show()
