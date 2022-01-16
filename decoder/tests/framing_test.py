import hypothesis
import pytest

import collections.abc

from decoder import framing


@hypothesis.given(
    # Generate bytes that don't contain b"T"
    data=hypothesis.strategies.lists(
        hypothesis.strategies.integers(0, 254).map(lambda x: x + int(x >= ord("T")))
    ).map(bytes)
)
def test_raw__keeps_normal_values(data):
    """Check that the raw framing decoder passes through non-escape bytes untouched."""
    assert list(framing.decode_framing_raw(data)) == list(data)


@hypothesis.given(data=hypothesis.strategies.lists(hypothesis.strategies.binary()))
def test_raw_joined_input_is_identical(data):
    """Check that the raw framing decoder outputs the same values when the input is chunked"""
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
    assert list(framing.decode_framing_raw(b"Tll")) == [
        framing.block_start_marker,
        ord("l"),
    ]


def flattened_framing(b):
    """Parses b using framing decoder and converts its output to a flattened byte
    string, that captures the complete structure of the decoded framing.
    To keep the flattened format simple, there is no escaping and only numbers are
    supposed to be used as the actual data inside the framed blocks"""
    output = []
    for item in framing.decode_framing(b):
        if isinstance(item, framing.UnexpectedCharacters):
            output.append(b"C")
        elif isinstance(item, framing.UnexpectedEndOfData):
            output.append(b"X")
        elif isinstance(item, framing.FramingError):
            output.append(b"E")
        else:
            assert isinstance(item, collections.abc.Iterator)
            output.append(b"(")
            output.append(bytes(item))
            output.append(b")")

    return b"".join(output)


def test_one_block():
    assert flattened_framing(b"Tl123T#") == b"(123)"


@hypothesis.strategies.composite
def flattened_block_decoding_inputs(draw):
    """This is a test strategy that generates a bytes input and an expected flattened
    output pairs"""
    data_strategy = hypothesis.strategies.text(alphabet="123", max_size=3).map(
        lambda x: x.encode("ascii")
    )
    junk_strategy = hypothesis.strategies.text(alphabet="123#", max_size=5).map(
        lambda x: x.replace("!", "T#").encode("ascii")
    )
    block_count = draw(hypothesis.strategies.integers(min_value=0, max_value=3))
    block_data = draw(
        hypothesis.strategies.lists(
            data_strategy, min_size=block_count, max_size=block_count
        )
    )

    junk_before = draw(junk_strategy)
    if block_count:
        gap_junk = draw(
            hypothesis.strategies.lists(
                junk_strategy, min_size=block_count - 1, max_size=block_count - 1
            )
        )
        incomplete_last_block = draw(hypothesis.strategies.booleans())
        if incomplete_last_block:
            junk_after = b""
        else:
            junk_after = draw(junk_strategy)

    input_parts = []
    expected_parts = []

    if len(junk_before):
        input_parts.append(junk_before)
        expected_parts.append(b"C")
    if block_count:
        for block, junk in zip(block_data[:-1], gap_junk):
            input_parts.append(b"Tl")
            input_parts.append(block)
            input_parts.append(b"T#")
            expected_parts.append(b"(")
            expected_parts.append(block)
            expected_parts.append(b")")

            if len(junk):
                input_parts.append(junk)
                expected_parts.append(b"C")

        input_parts.append(b"Tl")
        input_parts.append(block_data[-1])
        expected_parts.append(b"(")
        expected_parts.append(block_data[-1])
        expected_parts.append(b")")

        if incomplete_last_block:
            expected_parts.append(b"X")
        else:
            input_parts.append(b"T#")
            if len(junk_after):
                input_parts.append(junk_after)
                expected_parts.append(b"C")

    return (b"".join(input_parts), b"".join(expected_parts))


@hypothesis.given(flattened_block_decoding_inputs())
def test_block_decoding(data):
    input_data, expected_flattened = data
    assert flattened_framing(input_data) == expected_flattened
