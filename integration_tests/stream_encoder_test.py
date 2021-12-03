import hypothesis

from decoder import decoder_utils


def _br(binary_data):
    return decoder_utils.BitReader([binary_data])


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_byte_pattern(stream_encoder, length):
    """ Mostly testing the stream encoder RPC mechanism.
    Checks that generated bytes with increasing value are loaded as expected """
    expected = bytes(i & 0xFF for i in range(length))
    assert stream_encoder.call("byte_pattern", length) == expected


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_bit_pattern(stream_encoder, length):
    """Test the bit_pattern function, with an explicit expected value
    (not using bit reader from decoder)."""
    expected = b"\xaa" * (length // 8)
    remaining_bits = length - 8 * len(expected)
    if remaining_bits:
        expected += bytes([0xAA & ((1 << remaining_bits) - 1)])
    assert stream_encoder.call("bit_pattern", length) == expected


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024))
def test_bit_pattern2(stream_encoder, length):
    """Test the bit_pattern2 function, stressing the bit encoder and decoder a little. """
    encoded = stream_encoder.call("bit_pattern2", length)
    br = _br(encoded)
    decoded = [br.read(5) for _ in range(length)]
    expected = [i & 0x1f for i in range(length)]

    assert decoded == expected


@hypothesis.given(data=hypothesis.strategies.data())
def test_bit_encode_decode(stream_encoder, data):
    """Test bit writer and bit reader by serializing and deserializing random data"""
    wordsize = data.draw(hypothesis.strategies.sampled_from([8, 16, 32, 64]))
    blocks = [
        (data.draw(hypothesis.strategies.integers(0, 2**bit_len - 1)), bit_len)
        for bit_len in data.draw(
            hypothesis.strategies.lists(
                hypothesis.strategies.integers(0, wordsize),
                max_size=100
            )
        )
    ]

    encoded = stream_encoder.call(
        "write_bits",
        f"u{wordsize}",
        *[x for block in blocks for x in block]
    )

    br = _br(encoded)
    for (expected, bit_count) in blocks:
        assert br.read(bit_count) == expected


@hypothesis.given(data=hypothesis.strategies.data())
def test_elias_gamma(stream_encoder, data):
    """Test that elias gama encoder and decoder are inverse of each other."""
    wordsize = data.draw(
        hypothesis.strategies.sampled_from([8, 16, 32, 64]),
        label="word size"
        )
    value = data.draw(
        hypothesis.strategies.integers(0, 2**wordsize - 1),
        label="value"
    )

    encoded = stream_encoder.call("elias_gamma", f"u{wordsize}", value)
    decoded = decoder_utils.decode_elias_gamma(_br(encoded))
    assert decoded == value


@hypothesis.given(data=hypothesis.strategies.data())
def test_adaptive_exp_golomb(stream_encoder, data):
    """Test the adaptive number format by serializing and deserializing random data"""
    wordsize = data.draw(hypothesis.strategies.sampled_from([8, 16, 32, 64]))
    values = data.draw(
        hypothesis.strategies.lists(
            hypothesis.strategies.integers(0, 2**wordsize - 1),
            max_size=1024
        )
    )

    encoded = stream_encoder.call(
        "adaptive_exp_golomb",
        f"u{wordsize}",
        *[x for x in values]
    )

    br = _br(encoded)
    adaptive_exp_golomb_decoder = decoder_utils.AdaptiveExpGolombDecoder(wordsize)
    decoded = [adaptive_exp_golomb_decoder.decode(br) for _ in range(len(values))]

    assert decoded == values
