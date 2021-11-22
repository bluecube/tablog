import pytest


@pytest.mark.parametrize("length", [0, 1, 10, 258])
def test_byte_pattern(stream_encoder, length):
    assert stream_encoder.call("byte_pattern", length) == bytes(i & 0xff for i in range(length))
