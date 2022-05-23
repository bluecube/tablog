from __future__ import annotations

from . import decoder_utils
from . import bit_reader
from . import framing
from . import predictors
from . import exceptions

import collections.abc


class TablogDecoder:
    supported_version = 0  # Supported version of Tablog format

    def __init__(self, chunks: collections.abc.Iterable[bytes]):
        """Construct the decoder object, pass in the encoded data
        (either as single block or iterable of chunks)
        chunks: Iterable of bytes (or bytes) containing the compressed data.
        """

        self._framing_it = framing.decode_framing(chunks)
        self._bit_reader = None
        self.field_names = None
        self.field_types = None
        self._predictors = None
        self._error_decoders = None

        if not self._next_block():
            raise exceptions.InputEmptyError("No data to decode");

    def _next_block(self):
        try:
            item = next(self._framing_it)
        except StopIteration:
            return False

        if isinstance(item, collections.abc.Iterator):
            self._bit_reader = bit_reader.BitReader(item)
        else:
            assert isinstance(item, framing.FramingError)
            raise item

        old_field_names = self.field_names
        old_field_types = self.field_types

        self._read_header()

        if old_field_names is not None and old_field_names != self.field_names:
            raise exceptions.TablogError("Field names have changed")
        if old_field_types is not None and old_field_types != self.field_types:
            raise exceptions.TablogError("Field types have changed")

        return True

    def _read_header(self):
        version = decoder_utils.decode_elias_gamma(self._bit_reader)
        if version != self.supported_version:
            raise exceptions.UnsupportedVersionError(
                f"Input uses file format version {version}, we support {self.supported_version}"
            )

        field_count = decoder_utils.decode_elias_gamma(self._bit_reader) + 1

        self.field_names = []
        self.field_types = []
        self._predictors = []
        self._error_decoders = []
        for i in range(field_count):
            name = decoder_utils.decode_string(self._bit_reader)
            value_type = decoder_utils.decode_type(self._bit_reader)

            predictor = predictors.Adapt(
                value_type, 8, predictors.Last.factory(), predictors.LinearO2.factory()
            )
            error_decoder = decoder_utils.AdaptiveExpGolombDecoder(value_type.bitsize)

            self.field_names.append(name)
            self.field_types.append(value_type)
            self._predictors.append(predictor)
            self._error_decoders.append(error_decoder)

    def _read_value_error(self, error_decoder):
        hit = self._bit_reader.read_bit()

        if hit:
            return 0
        else:
            prediction_high = self._bit_reader.read_bit()
            error = error_decoder.decode(self._bit_reader) + 1

            return error if prediction_high else -error

    def _read_value(self, predictor, error_decoder):
        error = self._read_value_error(error_decoder)
        value = predictor.predict() - error
        predictor.feed(value)
        return value

    def _read_row(self):
        return [
            self._read_value(predictor, error_decoder)
            for predictor, error_decoder in zip(self._predictors, self._error_decoders)
        ]

    def __iter__(self):
        while True:
            while not self._bit_reader.end_of_block():
                yield self._read_row()
            if not self._next_block():
                return
