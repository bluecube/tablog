#!/bin/env python3

import math
import operator

from datasets import individual_datasets
from decoder import predictors

def evaluate_predictor_dataset(predictor, dataset):
    count = 0
    score = 0
    type_bits = int(dataset.field_types[0][1:])
    for x in dataset.data_iterator:
        x = x[0]
        error = abs(x - predictor.predict())
        count += 1
        score += math.floor(math.log2(abs(error) + 1)) + 1
        predictor.feed(x)

    return score, count

def evaluate_predictors(*predictor_factories):
    predictor_results = {}
    dataset_results = {}

    for predictor_factory in predictor_factories:
        predictor = predictor_factory.__name__
        print(f"{predictor}:")
        score_sum = 0
        total_count = 0
        for dataset in individual_datasets(include_synthetic=False):
            assert len(dataset.field_types) == 1
            score, count = evaluate_predictor_dataset(
                predictor_factory(dataset.field_types[0]),
                dataset
            )
            avg_score = score / count
            percent_score = 100 * avg_score / int(dataset.field_types[0][1:])
            print(f"  {dataset.name}: {avg_score:.1f} ({percent_score:.0f}%)")
            score_sum += score
            total_count += count

            try:
                dataset_score_sum, dataset_total_count = dataset_results[dataset.name]
            except KeyError:
                dataset_score_sum = 0
                dataset_total_count = 0
            dataset_score_sum += score
            dataset_total_count += count
            dataset_results[dataset.name] = (dataset_score_sum, dataset_total_count)

        predictor_results[str(predictor)] = score_sum / total_count

    print()
    print("Total:")

    for dataset_name, (score_sum, total_count) in dataset_results.items():
        avg_score = score_sum / total_count
        print(f"  {dataset_name}: {avg_score:.1f}")

    print()
    for predictor_name, avg_score in sorted(predictor_results.items(), key=operator.itemgetter(1)):
        print(f"  {predictor_name}: {avg_score:.1f}")


evaluate_predictors(
    predictors.Linear.factory(1),
    predictors.Linear.factory(2),
    predictors.Linear.factory(3),
    predictors.Linear.factory(4),
    predictors.Linear.factory(5),
    predictors.Linear.factory(10),
    predictors.LSTSQQuadratic3,
    predictors.LSTSQQuadratic4,
    predictors.LSTSQQuadratic5,
    #predictors.GeneralizedEWMA(1, 0.9),
    #predictors.GeneralizedEWMA(3, 0.9),
)
