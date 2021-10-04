#!/bin/env python3

import math
import operator
import itertools

from datasets import individual_datasets
from decoder import predictors


def evaluate_predictor_dataset(predictor, dataset):
    skip_first_avg = 10
    count = 0
    sum_bits = 0
    sum_abs_error = 0
    for x in dataset.data_iterator:
        x = x[0]
        error = abs(x - predictor.predict())
        count += 1
        sum_bits += math.floor(math.log2(error + 1)) + 1
        if count > skip_first_avg:
            sum_abs_error += error
        predictor.feed(x)

    sum_abs_error += skip_first_avg * sum_abs_error / (count - skip_first_avg)

    return sum_bits, sum_abs_error, count


def open_datasets():
    yield from individual_datasets(include_synthetic=False)


def evaluate_predictors(*predictor_factories):
    results = {}
    predictors = []
    datasets = [dataset.name for dataset in open_datasets()]

    for predictor_factory in predictor_factories:
        predictor = predictor_factory.__name__
        predictors.append(predictor)

        for dataset in open_datasets():
            assert len(dataset.field_types) == 1
            results[dataset.name, predictor] = evaluate_predictor_dataset(
                predictor_factory(dataset.field_types[0]), dataset
            )

    table_header = ["Dataset"]
    table_header.extend(predictors)
    table_header.append("Average")

    table_data = []
    for dataset_name in datasets:
        row = [dataset_name]
        total_sum_bits = 0
        total_sum_abs_error = 0
        total_count = 0
        for predictor_name in predictors:
            sum_bits, sum_abs_error, count = results[dataset_name, predictor_name]
            total_sum_bits += sum_bits
            total_sum_abs_error += sum_abs_error
            total_count += count
            row.append(f"{sum_abs_error / count:.1f}; {sum_bits / count:.1f}b")
        row.append(f"{total_sum_abs_error / total_count:.1f}; {total_sum_bits / total_count:.1f}b")
        table_data.append(row)

    row = ["Average"]
    for predictor_name in predictors:
        total_sum_bits = 0
        total_sum_abs_error = 0
        total_count = 0
        for dataset_name in datasets:
            sum_bits, sum_abs_error, count = results[dataset_name, predictor_name]
            total_sum_bits += sum_bits
            total_sum_abs_error += sum_abs_error
            total_count += count
        row.append(f"{total_sum_abs_error / total_count:.1f}; {total_sum_bits / total_count:.1f}b")
    row.append("")
    table_data.append(row)

    column_widths = [len(x) for x in table_header]
    for row in table_data:
        assert len(column_widths) == len(row)
        column_widths = [max(w, len(s)) for w, s in zip(column_widths, row)]

    def tag(s, w, tag_name):
        return f"<{tag_name}>{s}</{tag_name}>".rjust(w + 5 + 2 * len(tag_name))

    print("<table><thead>")
    print("<tr>", end="")
    for h, w in zip(table_header, column_widths):
        print(tag(h, w, "th"), end="")
    print("</tr>")
    print("</thead><tbody>")
    for row in table_data:
        print("<tr>", end="")
        print(tag(row[0], column_widths[0], "th"), end="")
        for s, w in zip(row[1:], column_widths[1:]):
            print(tag(s, w, "td"), end="")
        print("</tr>")
    print("</tbody></table>")


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
    # predictors.GeneralizedEWMA(1, 0.9),
    # predictors.GeneralizedEWMA(3, 0.9),
)
