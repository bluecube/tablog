import decoder.int_type

import pytest
import hypothesis
import struct

from . import strategies


@hypothesis.given(int_type=strategies.int_types())
def test_string_construction(int_type):
    """ Tests that converting a type to string and back gives the same type """
    s = str(int_type)
    assert int_type == decoder.int_type.IntType.from_string(s)


@pytest.mark.parametrize(
    "type_str,minimum,maximum",
    [
        ("u8", 0, 255),
        ("s8", -128, 127),
        ("u16", 0, 65535)
    ]
)
def test_min_max_examples(type_str, minimum, maximum):
    """ Tests minimum and maximum value calculation on simple examples """
    t = decoder.int_type.IntType.from_string(type_str)
    assert t.min() == minimum
    assert t.max() == maximum


@pytest.mark.parametrize("s", ["asdf", "x16", "s32x", "u12"])
def test_bad_from_string(s):
    with pytest.raises(ValueError):
        assert decoder.int_type.IntType.from_string(s)


@hypothesis.given(int_type=strategies.int_types())
def test_range_min_max(int_type):
    """ Tests that range and min/max return the same values """
    r = int_type.range()
    assert r.start == int_type.min()
    assert r.stop == int_type.max() + 1


@hypothesis.given(
    int_type=strategies.int_types(),
    value=hypothesis.strategies.integers()
)
def test_clamp(int_type, value):
    clamped = int_type.clamp(value)
    assert clamped in int_type.range()


@hypothesis.given(
    signed=strategies.int_type_values(decoder.int_type.IntType.from_string("s16"))
)
def test_convert_unsigned_s16(signed):
    """ Check converting s16 value from unsigned against python struct module """
    unsigned = struct.unpack("=H", struct.pack("=h", signed))[0]
    converted = decoder.int_type.IntType.from_string("s16").convert_unsigned(unsigned)
    assert converted == signed


@hypothesis.given(param=strategies.typed_values(unsigned_only=True))
def test_convert_unsigned_from_unsigned(param):
    """ Check that converting a value from unsigned to unsigned doesn't change it """
    converted = param[0].convert_unsigned(param[1])
    assert converted == param[1]
