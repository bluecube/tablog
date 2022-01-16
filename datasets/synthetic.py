from . import dataset
from decoder import int_type

import math
import itertools
import random as random_mod
import inspect
import sys
import hashlib
import functools


def all_datasets(length):
    types = [int_type.IntType.from_string(s) for s in ["u8", "s16", "u64", "s64"]]
    # u8 is easy to analyze by looking at it, u64 and s64 have max range,
    # s16 is just something in between.

    if length == 0:
        # Don't generate many identical datasets
        for t in types:
            yield dataset.Dataset(f'empty("{t}")', ["value"], [t], lambda: iter([]), 0)
        return

    dataset_name_suffix = f", length={length})"
    currentmod = sys.modules[__name__]

    for func_name, func in inspect.getmembers(currentmod, inspect.isfunction):
        if func_name.startswith("_"):
            continue
        if func is all_datasets:
            continue
        for t in types:
            dataset_name_prefix = f'{func_name}("{t}"'
            if "period" in inspect.signature(func).parameters:
                for period in [100, 10000]:
                    yield dataset.Dataset(
                        f"{dataset_name_prefix}, {period}{dataset_name_suffix}",
                        ["value"],
                        [t],
                        functools.partial(func, t, period, length),
                        length,
                    )
            else:
                yield dataset.Dataset(
                    dataset_name_prefix + dataset_name_suffix,
                    ["value"],
                    [t],
                    functools.partial(func, t, length),
                    length,
                )


def _remap(f, input_min, input_max, x_scale, t, length):
    """Turn a function giving a value from <input_min, input_max> to an infinite iterator of
    integers between <0, 2**scale_bits> (signed == "u") or
    <-2**(scale_bits - 1), 2**(scale_bits - 1) - 1> (signed == "s")"""
    low, high = t.minmax()

    y_scale = (high - low) / (input_max - input_min)
    y_offset = low - input_min * y_scale

    return (
        [max(low, min(round(f(i * x_scale) * y_scale + y_offset), high))]
        for i in range(length)
    )


def sine(t, period, length):
    return _remap(math.sin, -1, 1, 2 * math.pi / period, t, length)


def sawtooth(t, period, length):
    return _remap(lambda x: math.modf(x)[0], 0, 1, 1 / period, t, length)


def minor7chord(t, period, length):
    return _remap(
        lambda x: math.sin(x)
        + math.sin(x * 1.2)
        + math.sin(x * 1.5)
        + math.sin(x * 1.8),
        -4,
        4,
        2 * math.pi / period,
        t,
        length,
    )


def _random(gen, t, length):
    r = t.range()
    return ([int(gen.randrange(r.start, r.stop))] for _ in range(length))


def _make_generator(*args):
    hasher = hashlib.sha1()
    hasher.update(repr(args).encode())
    seed = int.from_bytes(hasher.digest(), byteorder="big")
    return random_mod.Random(seed)


def random_step(t, period, length):
    gen = _make_generator(t, period)
    remaining = length
    for value in _random(gen, t, length):
        segment_count = min(period // 2 + _geometric(gen, 2 / period), remaining)
        remaining = remaining - segment_count

        yield from itertools.repeat(value, segment_count)

        if remaining == 0:
            break


def random_smooth(t, period, length):
    gen = _make_generator(t, period)
    remaining = length
    it = _random(gen, t, length + 4)

    prev = next(it)[0]
    while remaining:
        current = next(it)[0]
        segment_count_raw = period // 2 + _geometric(gen, 2 / period)
        segment_count = min(segment_count_raw, remaining)
        remaining = remaining - segment_count

        # t = (1 - math.cos(i * math.pi / segment_count_raw)) / 2
        # v = prev + t * (current - prev)

        a = math.pi / segment_count_raw
        b = 0.5 * (prev + current)
        c = 0.5 * (current - prev)

        yield from ([round(b - math.cos(i * a) * c)] for i in range(segment_count))

        prev = current


def random(t, length):
    return _random(_make_generator(t), t, length)


def count_up(t, length):
    low, high = t.minmax()
    scale = high - low
    return ([(i % scale) + low] for i in range(length))


def unexpected_jump(t, period, length):
    """Generate a value close to zero, with an occasional jump to maximum value.

    Dataset like this is a critical failure point for vanilla Golomb encoder
    (the unexpected jump will cause a large missprediction, that gets encoded to
    approximately as many bits as is the maximum value of the variable)."""
    gen = _make_generator(t, period)
    next_jump = _geometric(gen, 1 / period)
    for i in range(length):
        if i == next_jump:
            yield [t.max()]
            next_jump += _geometric(gen, 1 / period)
        else:
            yield [int(gen.randint(10, 20))]


def _geometric(gen, p):
    # Might be wrong, but it's still ok.
    return int(math.log(1.0 - gen.random()) / math.log(1.0 - p))
