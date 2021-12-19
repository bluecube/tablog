import pytest
import hypothesis

from decoder import decoder_utils


@pytest.fixture(
    params=[
        lambda x: x,
        lambda x: [x],
        lambda x: [x[i:i+1] for i in range(len(x))],
    ],
    ids=[
        "raw-bytes",
        "single-item-list",
        "individual-bytes",
    ],
    scope="session"
)
def data_wrapper(request):
    return request.param


@hypothesis.given(length=hypothesis.strategies.integers(1, 1024 * 8))
def test_bit_pattern(length, data_wrapper):
    data = b"\xaa" * (length // 8)
    remaining_bits = length - 8 * len(data)
    if remaining_bits:
        data += bytes([0xAA & ((1 << remaining_bits) - 1)])

    br = decoder_utils.BitReader(data_wrapper(data))

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
    br = decoder_utils.BitReader(data)

    assert br.read(8 * len(joined_data)) == int.from_bytes(joined_data, "little", signed=False)


@pytest.mark.parametrize(
    "data,expected_reads",
    [
        pytest.param(
            b"\x09",
            [(0x9, 4)],
            id="4 bits"
        ),
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
def test_examples_matching_cpp(data, expected_reads, data_wrapper):
    """ Test manually defined values and expected results matching the C++ bit writer test. """
    br = decoder_utils.BitReader(data_wrapper(data))

    for (expected, bit_count) in expected_reads:
        assert br.read(bit_count) == expected


@pytest.mark.parametrize(
    "data,expected",
    [
        (b"\x01", 0),
        (b"\x02", 1),
        (b"\x06", 2),
    ]
)
def test_decode_elias_gamma_manual(data, expected):
    br = decoder_utils.BitReader([data])
    assert decoder_utils.decode_elias_gamma(br) == expected
