import pytest
import datasets
import csv_tablog_runner
from decoder import predictors


# Mapping C++ predictor names to python predictor factory
cpp_equivalents = {
    "linear3": predictors.Linear.factory(3),
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
    ids=lambda pair: pair[0]
)
@pytest.mark.parametrize(
    "dataset",
    list(datasets.individual_datasets(True)),
    ids=lambda dataset: dataset.name
)
def test_predictor_equality(predictor, dataset):
    predictor_name, predictor_factory = predictor
    predictor = predictor_factory(dataset.field_types[0])
    for (v,), cpp_error in zip(dataset, _cpp_errors(dataset, predictor_name)):
        prediction = predictor.predict_and_feed(v)
        error = prediction - v

        assert error == cpp_error
