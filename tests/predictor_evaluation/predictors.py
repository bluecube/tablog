#!/bin/env python3

import math
import operator

from datasets import individual_datasets
from decoder import predictors

def evaluate_predictor_dataset(predictor, dataset_type, dataset):
    count = 0
    score = 0
    type_bits = int(dataset_type[1:])
    for x in dataset:
        error = abs(x - predictor.predict())
        count += 1
        score += math.log2(abs(error) + 1)
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
        for dataset_name, dataset_type, dataset in individual_datasets(synthetic=False):
            score, count = evaluate_predictor_dataset(
                predictor_factory(dataset_type),
                dataset_type,
                dataset
            )
            avg_score = score / count
            percent_score = 100 * avg_score / int(dataset_type[1:])
            print(f"  {dataset_name}: {avg_score:.1f} ({percent_score:.0f}%)")
            score_sum += score
            total_count += count

            try:
                dataset_score_sum, dataset_total_count = dataset_results[dataset_name]
            except KeyError:
                dataset_score_sum = 0
                dataset_total_count = 0
            dataset_score_sum += score
            dataset_total_count += count
            dataset_results[dataset_name] = (dataset_score_sum, dataset_total_count)

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
    predictors.SimpleLinearPredictor.factory(1),
    predictors.SimpleLinearPredictor.factory(2),
    predictors.SimpleLinearPredictor.factory(3),
    predictors.SimpleLinearPredictor.factory(4),
    predictors.SimpleLinearPredictor.factory(5),
    predictors.SimpleLinearPredictor.factory(10),
    predictors.ThreePointQuadraticPredictor.factory(),
    predictors.FourPointQuadraticPredictor.factory(),
    predictors.FivePointQuadraticPredictor.factory(),
    #predictors.GeneralizedEWMA(1, 0.9),
    #predictors.GeneralizedEWMA(3, 0.9),
)
