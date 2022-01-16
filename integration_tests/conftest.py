import tools.subprocess_iterator
import tools.encoder_wrappers

import pytest


@pytest.fixture
def csv_encoder():
    """Provides a callable that uses the csv_encoder mechanism to encode a dataset."""
    return tools.encoder_wrappers.csv_encoder


@pytest.fixture(scope="session")
def stream_encoder():
    with tools.encoder_wrappers.StreamEncoder() as se:
        yield se
