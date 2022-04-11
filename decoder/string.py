from . import decoder_utils

# The following code was generated from the jupyter notebook string_predictors.ipynb
# fmt: off
_decoder_trie = (
    (((b'i', ((b'g', (b'ed', (b'tion', b'tio'))), b'd')), (b'o',
    ((b'in', (b'te', (((b'ht', (((((b'htt', b'.c'), b':'), (b':/',
    b';')), b'.co'), b',')), b'http'), b'.'))), (((b'://', b';\n'),
    b', '), (b'-', b'. '))))), ((b'a', ((b'p', (b'or', b'v')), b'l')),
    (((((b'ce', b'ne'), b'ti'), (b'es', b'nd')), (((b'ea', b'ra'),
    (b'co', b'ic')), ((b'ro', b'ion'), b'en'))), b't'))), (((((b'f',
    (((b'atio', b'ati'), b'ri'), (b'de', b'hi'))), (b'he', (b'at',
    (b'ing', b'me')))), ((b'm', b' '), b'_')), ((b'h', (b'th', (b'1',
    (b've', b'le')))), ((((b'io', b'as'), b'b'), b'u'), ((b'on',
    (b'ou', b'se')), (b're', (b'ha', b'and')))))), ((b'e', ((((b'ng',
    (b'.com', b'ent')), b'an'), (b'w', (b'st', b'to'))), b'r')),
    ((b's', (b'c', (b'er', b'y'))), ((((b'nt', b'ar'), (b'al',
    b'it')), (b'the', (b'is', b'of'))), b'n'))))
)  # noqa: E128
# fmt: on


def _decode_non_match_group(count, bit_reader):
    return (bytes([bit_reader.read(8)]) for _ in range(count))


def _decode_match_group(count, bit_reader):
    for i in range(count):
        current = _decoder_trie
        while True:
            bit = bit_reader.read_bit()
            current = current[bit]
            if isinstance(current, bytes):
                yield current
                break


def _decode_string_parts(bit_reader):
    count = decoder_utils.decode_elias_gamma(bit_reader)
    if count > 0:
        yield from _decode_match_group(count, bit_reader)

    while True:
        count = decoder_utils.decode_elias_gamma(bit_reader)
        if not count:
            return
        yield from _decode_non_match_group(count, bit_reader)

        count = decoder_utils.decode_elias_gamma(bit_reader)
        if not count:
            return
        yield from _decode_match_group(count, bit_reader)


def decode_string(bit_reader):
    return b"".join(_decode_string_parts(bit_reader))
