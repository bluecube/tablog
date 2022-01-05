import hypothesis
import pytest

from decoder import framing


@hypothesis.given(
    # Generate bytes that don't contain b"T"
    data=hypothesis.strategies.lists(
        hypothesis.strategies.integers(0, 254).map(lambda x: x + int(x >= ord("T")))
    ).map(bytes)
)
def test_raw__keeps_normal_values(data):
    """ Check that the raw framing decoder passes through non-escape bytes untouched. """
    assert list(framing.decode_framing_raw(data)) == list(data)


@hypothesis.given(data=hypothesis.strategies.lists(hypothesis.strategies.binary()))
def test_raw_joined_input_is_identical(data):
    """ Check that the raw framing decoder outputs the same values when the input is chunked """
    segmented = list(framing.decode_framing_raw(data))
    joined = list(framing.decode_framing_raw(b"".join(data)))
    assert segmented == joined


def test_raw_start_block():
    assert list(framing.decode_framing_raw(b"Tl")) == [framing.block_start_marker]


def test_raw_end_block():
    assert list(framing.decode_framing_raw(b"T#")) == [framing.block_end_marker]


def test_raw_double_escape():
    assert list(framing.decode_framing_raw(b"T ")) == list(b"T")


def test_raw_escape_unused():
    assert list(framing.decode_framing_raw(b"Tx")) == list(b"Tx")


def test_raw_two_escape():
    assert list(framing.decode_framing_raw(b"TT")) == list(b"TT")


def test_raw_two_commands():
    assert list(framing.decode_framing_raw(b"Tll")) == [framing.block_start_marker, ord("l")]
