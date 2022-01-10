import pytest
import hypothesis

from decoder import bit_reader


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_bit_pattern(length):
    """ Check that artificially generated alternating bit pattern gets correctly
    parsed from the input bytes and that the end flag is correctly recognized """
    data = b"\xaa" * (length // 8)
    remaining_bits = length - 8 * len(data)
    if remaining_bits:
        byte = 0xAA & ((1 << remaining_bits) - 1)
        byte |= 1 << remaining_bits  # End marker
        data += bytes([byte])
    else:
        data += b"\x01"  # End marker

    br = bit_reader.BitReader(data)

    expected = [(i & 1) for i in range(length)]

    bits = [br.read_bit() for _ in range(length - 1)]
    assert not br.end_of_block()
    bits.append(br.read_bit())
    assert br.end_of_block()

    assert bits == expected

    with pytest.raises(bit_reader.IncompleteRead) as exc_info:
        br.read_bit()
    assert exc_info.value.remaining == 0


@hypothesis.given(data=hypothesis.strategies.binary(max_size=1024))
def test_single_long_integer(data):
    br = bit_reader.BitReader(data + b"\x01")  # Data + end marker
    assert br.read(8 * len(data)) == int.from_bytes(data, "little", signed=False)


@pytest.mark.parametrize(
    "data,expected_reads",
    [
        pytest.param(
            b"\x19",
            [(0x9, 4)],
            id="4 bits"
        ),
        pytest.param(
            b"\x76\x98\x01",
            [(0x6, 4), (0x7, 4), (0x8, 4), (0x9, 4)],
            id="4x4 bits"
        ),
        pytest.param(
            b"\xc7\xf2\x82\x13\x01",
            [(62151, 16), (4994, 16)],
            id="2x16 bits"
        ),
        pytest.param(
            b"\xc7\xf2\x82\x13\x01",
            [(327348935, 32)],
            id="1x32 bits"
        ),
        pytest.param(
            b"\x8e\xe5\x05\x27\x03",
            [(0, 1), (2474832583, 32)],
            id="1 + 32 bits"
        ),
    ]
)
def test_examples_matching_cpp(data, expected_reads):
    """ Test manually defined values and expected results matching the C++ bit writer test.
    Hovewer the test data include the end flags, which in the C++ test they dont. """

    br = bit_reader.BitReader(data)

    for (expected, bit_count) in expected_reads:
        assert br.read(bit_count) == expected


@hypothesis.given(nbits=hypothesis.strategies.integers(min_value=1))
def test_empty_end(nbits):
    br = bit_reader.BitReader(b"\x01")
    assert br.end_of_block()

    with pytest.raises(bit_reader.IncompleteRead) as exc_info:
        br.read(nbits)
    assert exc_info.value.remaining == 0


def test_too_large_read():
    br = bit_reader.BitReader(b"\x03")
    with pytest.raises(bit_reader.IncompleteRead) as exc_info:
        br.read(2)
    assert exc_info.value.remaining == 1
    assert exc_info.value.nbits == 2
