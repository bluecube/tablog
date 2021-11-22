import pytest
import datasets
import csv_tablog_runner
from decoder import predictors


# Mapping C++ predictor names to python predictor factory
cpp_equivalents = {
    "linear3": predictors.Linear.factory(3),
    "linear12adapt":
        predictors.Adapt.factory(
            8,
            predictors.Last.factory(),
            predictors.LinearO2.factory()
        ),
}


def _cpp_errors(dataset, predictor_name):
    for record in csv_tablog_runner.encode_record_stream(dataset, predictor_name):
        try:
            yield record["error"]
        except KeyError:
            pass


@pytest.mark.parametrize(
    "predictor",
    cpp_equivalents.items(),
    ids=cpp_equivalents.keys()
)
@pytest.mark.parametrize(
    "dataset",
    [
        pytest.param(
            dataset,
            id=dataset.name,
            marks=pytest.mark.slow if dataset.length is None or dataset.length > 100 else []
        )
        for dataset in datasets.individual_datasets(True)
    ]
)
def test_equality(predictor, dataset):
    """ Check that python and C++ predictors generate the same values """
    predictor_name, predictor_factory = predictor
    predictor = predictor_factory(dataset.field_types[0])
    for (v,), cpp_error in zip(dataset, _cpp_errors(dataset, predictor_name)):
        prediction = predictor.predict_and_feed(v)
        error = prediction - v

        assert error == cpp_error
