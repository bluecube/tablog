import pytest
import hypothesis

from decoder import decoder_utils
from decoder import bit_reader
from decoder import framing
from decoder import predictors
from decoder.tests import strategies


def _br(binary_data):
    return bit_reader.BitReader(binary_data)


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_byte_pattern(stream_encoder, length):
    """Mostly testing the stream encoder RPC mechanism.
    Checks that generated bytes with increasing value are loaded as expected"""
    expected = bytes(i & 0xFF for i in range(length))
    assert stream_encoder.call("byte_pattern", length) == expected


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_bit_pattern(stream_encoder, length):
    """Test the bit_pattern function, with an explicit expected value
    (not using bit reader from decoder)."""
    expected = b"\xaa" * (length // 8)
    remaining_bits = length - 8 * len(expected)
    if remaining_bits:
        byte = 0xAA & ((1 << remaining_bits) - 1)
        byte |= 1 << remaining_bits  # End marker
        expected += bytes([byte])
    else:
        expected += b"\x01"  # End marker

    assert stream_encoder.call("bit_pattern", length) == expected


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024))
def test_bit_pattern2(stream_encoder, length):
    """Test the bit_pattern2 function, stressing the bit encoder and decoder a little."""
    encoded = stream_encoder.call("bit_pattern2", length)
    br = _br(encoded)
    decoded = [br.read(5) for _ in range(length)]
    expected = [i & 0x1F for i in range(length)]

    assert decoded == expected

    with pytest.raises(bit_reader.IncompleteRead) as exc_info:
        br.read_bit()
    assert exc_info.value.remaining == 0


@hypothesis.given(data=hypothesis.strategies.data())
def test_bit_encode_decode(stream_encoder, data):
    """Test bit writer and bit reader by serializing and deserializing random data"""
    int_type = data.draw(strategies.int_types(unsigned_only=True))
    blocks = [
        (data.draw(hypothesis.strategies.integers(0, 2 ** bit_len - 1)), bit_len)
        for bit_len in data.draw(
            hypothesis.strategies.lists(
                hypothesis.strategies.integers(0, int_type.bitsize), max_size=100
            )
        )
    ]

    encoded = stream_encoder.call(
        "write_bits", str(int_type), *[x for block in blocks for x in block]
    )

    br = _br(encoded)
    for (expected, bit_count) in blocks:
        assert br.read(bit_count) == expected


@hypothesis.given(
    data=hypothesis.strategies.text(alphabet="Tl# x12").map(lambda x: x.encode("utf-8"))
)
def test_framing_raw(stream_encoder, data):
    """Test that framing encoding and decoding restores input and control commands correctly

    x is used as a placeholder for a generic non-special character
    1, 2 are used as a placeholder for start and stop flags.
    """
    encoded = stream_encoder.call("framing", data)

    decoded = b""
    for b in framing.decode_framing_raw(encoded):
        if b is framing.block_start_marker:
            decoded += b"1"
        elif b is framing.block_end_marker:
            decoded += b"2"
        else:
            decoded += bytes([b])

    assert decoded == data


@hypothesis.given(data=strategies.typed_values(unsigned_only=True))
def test_elias_gamma(stream_encoder, data):
    """Test that elias gama encoder and decoder are inverse of each other."""
    int_type, value = data
    encoded = stream_encoder.call("elias_gamma", str(int_type), value)
    decoded = decoder_utils.decode_elias_gamma(_br(encoded))
    assert decoded == value


@hypothesis.given(data=strategies.typed_lists(max_size=1024, unsigned_only=True))
def test_adaptive_exp_golomb(stream_encoder, data):
    """Test the adaptive number format by serializing and deserializing random data"""
    int_type, values = data

    encoded = stream_encoder.call(
        "adaptive_exp_golomb", str(int_type), *[x for x in values]
    )

    br = _br(encoded)
    adaptive_exp_golomb_decoder = decoder_utils.AdaptiveExpGolombDecoder(
        int_type.bitsize
    )
    decoded = [adaptive_exp_golomb_decoder.decode(br) for _ in range(len(values))]

    assert decoded == values


@hypothesis.given(int_type=strategies.int_types())
def test_encode_type(stream_encoder, int_type):
    encoded = stream_encoder.call("encode_type", str(int_type))
    decoded = decoder_utils.decode_type(_br(encoded))
    assert decoded == int_type


@hypothesis.given(int_type=strategies.int_types())
def test_type_info(stream_encoder, int_type):
    """Verify that types in C++ behave the same as IntType class."""
    encoded = stream_encoder.call("type_info", str(int_type))
    br = _br(encoded)

    signed = br.read_bit()
    assert signed == int_type.signed

    sizeof = decoder_utils.decode_elias_gamma(br)
    assert sizeof == int_type.bytesize()

    digits = decoder_utils.decode_elias_gamma(br)
    expected_digits = int_type.bitsize - int(signed)
    assert digits == expected_digits

    abs_min = decoder_utils.decode_elias_gamma(br)
    assert abs_min == abs(int_type.min())

    max = decoder_utils.decode_elias_gamma(br)
    assert max == int_type.max()


@pytest.mark.parametrize(
    "predictor",
    [
        ("linear3", predictors.Linear.factory(3)),
        (
            "linear12adapt",
            predictors.Adapt.factory(
                8, predictors.Last.factory(), predictors.LinearO2.factory()
            ),
        ),
    ],
    ids=lambda x: x[0],
)
@hypothesis.given(data=strategies.typed_lists(max_size=10000))
def test_predictors_equality(stream_encoder, predictor, data):
    """Check that python and C++ predictors generate the same values"""
    int_type, values = data

    encoded = stream_encoder.call(f"{predictor[0]}_predictor", str(int_type), *values)
    br = _br(encoded)

    decoded = [
        int_type.convert_unsigned(br.read(int_type.bitsize))
        for i in range(0, len(values))
    ]

    predictor = predictor[1](int_type)
    expected = []
    for value in values:
        expected.append(predictor.predict_and_feed(value))

    assert expected == decoded
