import pytest


@pytest.mark.parametrize("length", [0, 1, 10, 258])
def test_byte_pattern(stream_encoder, length):
    expected = bytes(i & 0xFF for i in range(length))
    assert stream_encoder.call("byte_pattern", length) == expected


@pytest.mark.parametrize("length", [0, 1, 3, 8, 12, 13, 14, 16, 1024])
def test_bit_pattern_explicit(stream_encoder, length):
    """Test the bit_pattern function, with an explicit expected value
    (not using bit reader from decoder)."""
    expected = b"\xaa" * (length // 8)
    remaining_bits = length - 8 * len(expected)
    if remaining_bits:
        expected += bytes([0xAA & ((1 << remaining_bits) - 1)])
    assert stream_encoder.call("bit_pattern", length) == expected
