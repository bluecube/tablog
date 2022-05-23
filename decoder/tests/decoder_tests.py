import pytest

import decoder


def test_decoding_empty_data_fails():
    with pytest.raises(decoder.exceptions.InputEmptyError):
        decoder.TablogDecoder(b"")
