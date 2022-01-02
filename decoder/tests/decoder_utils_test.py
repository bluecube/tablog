import pytest

from decoder import decoder_utils
from decoder import bit_reader


@pytest.mark.parametrize(
    "data,expected",
    [
        (b"\x03", 0),
        (b"\x0a", 1),
        (b"\x0e", 2),
    ]
)
def test_decode_elias_gamma_manual(data, expected):
    """ Decoding Elias gamma encoded numbers from manual examples.
    The examples already contain bit reader end mark """
    br = bit_reader.BitReader(data)
    assert decoder_utils.decode_elias_gamma(br) == expected
