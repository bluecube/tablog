from . import decoder_utils
from . import predictors
from . import int_type

from collections.abc import Iterable
from typing import Union


class TablogDecoder:
    supported_version = 0  # Supported version of Tablog format

    def __init__(self, chunks: Union[bytes, Iterable[bytes]]):
        """ Construct the decoder object, pass in the encoded data
        (either as single block or iterable of chunks)
        chunks: Iterable of bytes (or bytes) containing the compressed data.
        """
        self._bit_reader = decoder_utils.BitReader(chunks)
        self.field_names = None
        self.field_types = None
        self._predictors = None
        self._error_decoders = None

        self._read_header()

    def _read_header(self):
        version = decoder_utils.decode_elias_gamma(self._bit_reader)
        if version != self.supported_version:
            raise ValueError(
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
                value_type, 8,
                predictors.Last.factory(), predictors.LinearO2.factory()
            )
            error_decoder = decoder_utils.AdaptiveExpGolombDecoder(value_type.bitsize)

            self.field_names.append(name)
            self.field_types.append(value_type)
            self._predictors.append(predictor)
            self._error_decoders.append(error_decoder)

    def _read_value_error(self, error_decoder):
        miss = self._bit_reader.read_bit()
        if miss is None:
            # We ran out of encoded data, stop the row iterator
            # Raising an exception here is kind of hacky, but simple solution
            raise StopIteration
        if miss:
            prediction_high = self._bit_reader.read_bit()
            error = error_decoder.decode(self._bit_reader)

            return error if prediction_high else -error
        else:
            return 0

    def _read_value(self, predictor, error_decoder):
        error = self._read_value_error(error_decoder)
        ret = predictor.predict() - error
        predictor.feed(ret)

        return ret

    def __iter__(self):
        try:
            while True:
                yield [
                    self._read_value(predictor, error_decoder)
                    for predictor, error_decoder
                    in zip(self._predictors, self._error_decoders)
                ]
        except StopIteration:
            pass
