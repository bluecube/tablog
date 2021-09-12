#!/bin/env python3

import math
import copy
import operator

from datasets import individual_datasets
from decoder import predictors

def evaluate_predictor_dataset(predictor, dataset):
    count = 0
    score = 0
    for x in dataset:
        error = abs(x - predictor.predict())
        count += 1
        if error > 0:
            score += math.log2(error) + 1
        predictor.feed(x)

    return score, count

def evaluate_predictors(*predictors):
    predictor_results = {}
    #TODO: Dataset results

    for predictor in predictors:
        print(f"{predictor}:")
        score_sum = 0
        total_count = 0
        for dataset_name, dataset in individual_datasets():
            score, count = evaluate_predictor_dataset(copy.deepcopy(predictor), dataset)
            avg_score = score / count
            print(f"  {dataset_name}: {avg_score:.1f}")
            score_sum += score
            total_count += count

        predictor_results[str(predictor)] = score_sum / total_count

    print()
    print("Total:")
    for predictor_name, avg_score in sorted(predictor_results.items(), key=operator.itemgetter(1)):
        print(f"  {predictor_name}: {avg_score:.1f}")


evaluate_predictors(
    predictors.SimpleLinearPredictor(1),
    predictors.SimpleLinearPredictor(3),
    predictors.SimpleLinearPredictor(10),
    predictors.GeneralizedEWMA(1, 0.9),
    predictors.GeneralizedEWMA(3, 0.9),
)
