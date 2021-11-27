import pytest
import datasets
from decoder import predictors


# Mapping C++ predictor names to python predictor factory
cpp_equivalents = {
    "linear3": predictors.Linear.factory(3),
    "linear12adapt": predictors.Adapt.factory(
        8, predictors.Last.factory(), predictors.LinearO2.factory()
    ),
}


@pytest.mark.parametrize(
    "predictor", cpp_equivalents.items(), ids=cpp_equivalents.keys()
)
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
        for dataset in datasets.individual_datasets(True)
    ],
)
@pytest.mark.dataset
def test_equality(csv_encoder_json, predictor, dataset):
    """Check that python and C++ predictors generate the same values"""
    predictor_name, predictor_factory = predictor
    predictor = predictor_factory(dataset.field_types[0])

    cpp_errors = []
    for record in csv_encoder_json(dataset, predictor_name):
        print(record)
        try:
            cpp_errors.append(record["error"])
        except KeyError:
            pass

    python_errors = []
    for (v,) in dataset:
        prediction = predictor.predict_and_feed(v)
        error = prediction - v
        python_errors.append(error)

    assert cpp_errors == python_errors
