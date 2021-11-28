import pytest
import hypothesis

from decoder import bit_reader


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_bit_pattern(length):
    data = b"\xaa" * (length // 8)
    remaining_bits = length - 8 * len(data)
    if remaining_bits:
        data += bytes([0xAA & ((1 << remaining_bits) - 1)])

    br = bit_reader.BitReader([data])

    expected = [(i & 1) for i in range(length)]
    bits = [br.read_bit() for _ in range(length)]

    assert bits == expected


@hypothesis.given(
    data=hypothesis.strategies.lists(
        hypothesis.strategies.binary(max_size=1024),
        max_size=100
    )
)
def test_single_chunked_integer(data):
    joined_data = b"".join(data)
    br = bit_reader.BitReader(data)

    assert br.read(8 * len(joined_data)) == int.from_bytes(joined_data, "little", signed=False)


@pytest.mark.parametrize(
    "data,expected_reads",
    [
        pytest.param(
            b"\x76\x98",
            [(0x6, 4), (0x7, 4), (0x8, 4), (0x9, 4)],
            id="4x4 bits"
        ),
        pytest.param(
            b"\xc7\xf2\x82\x13",
            [(62151, 16), (4994, 16)],
            id="2x16 bits"
        ),
        pytest.param(
            b"\xc7\xf2\x82\x13",
            [(327348935, 32)],
            id="1x32 bits"
        ),
        pytest.param(
            b"\x8e\xe5\x05\x27\x01",
            [(0, 1), (2474832583, 32)],
            id="1 + 32 bits"
        ),
    ]
)
def test_examples_matching_cpp(data, expected_reads):
    """ Test manually defined values and expected results matching the C++ bit writer test. """
    br = bit_reader.BitReader([data])

    for (expected, bit_count) in expected_reads:
        assert br.read(bit_count) == expected
