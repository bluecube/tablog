import pytest
import datasets
import decoder as decoder_module


@pytest.mark.parametrize(
    "dataset",
    [
        pytest.param(
            dataset,
            id=dataset.name,
            marks=pytest.mark.slow
            if dataset.length is None or dataset.length > 100
            else [],
        )
        for dataset in datasets.all_datasets(include_synthetic=True)
    ],
)
@pytest.mark.dataset
def test_dataset_encode_decode(csv_encoder, dataset):
    """Check that encoding and decoding a dataset generates identical values"""

    try:
        encoded = csv_encoder(dataset)
        decoder = decoder_module.TablogDecoder(encoded)

        assert decoder.field_names == [name.encode("utf-8") for name in dataset.field_names]
        assert decoder.field_types == dataset.field_types

        for decoded_row, dataset_row in zip(decoder, dataset):
            assert decoded_row == dataset_row
    except csv_encoder.UnsupportedTypeSignature as e:  # pragma: no cover
        pytest.skip(str(e))
