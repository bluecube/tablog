#!/bin/env python3

import itertools
import operator
import math
import gzip
import io

from datasets import individual_datasets
from decoder import predictors


def encoded_length(abs_error):
    """ Encoded bit length of the error, based on the adaptive rice coding
    with no runs and sign bit. """
    if abs_error == 0:
        return 1  # Just hit flag
    else:
        raw_binary = math.floor(math.log2(abs_error)) + 1
        return raw_binary + 4  # miss flag, sign bit, 2 extra bits of overhead for number encoding


def evaluate_predictor_dataset(predictor, dataset):
    skip_first_avg = 10
    count = 0
    sum_bits = 0
    sum_abs_error = 0
    for x in dataset:
        x = x[0]
        error = abs(x - predictor.predict_and_feed(x))
        count += 1
        sum_bits += encoded_length(error)
        if count > skip_first_avg:
            sum_abs_error += error

    # Replace the skipped initial values in the sum with the average
    # sum_abs_error += skip_first_avg * sum_abs_error / (count - skip_first_avg)

    return sum_bits, sum_abs_error, count


def evaluate_gzip_dataset(dataset, diff):
    class FakeWriter(io.IOBase):
        def __init__(self):
            self.count = 0
        def write(self, b):
            self.count += len(b)

    byte_size = int(dataset.field_types[0][1:]) // 8
    signed = dataset.field_types[0][0] == "s"
    count = 0
    fake = FakeWriter()
    with gzip.open(fake, "wb") as gz:
        if diff:
            last_value = None
            for row in dataset:
                if last_value is not None:
                    gz.write((row[0] - last_value).to_bytes(byte_size + 1, "big", signed=True))
                    count += 1
                last_value = row[0]
        else:
            for row in dataset:
                gz.write(row[0].to_bytes(byte_size, "big", signed=signed))
                count += 1
    return (fake.count * 8, 0, count)


def open_datasets():
    yield from individual_datasets(include_synthetic=False)


def ranking(values):
    decorated = [(v, i) for i, v in enumerate(values)]
    decorated.sort()
    ret = [None] * len(decorated)
    for i, (_, group) in enumerate(itertools.groupby(decorated, key=operator.itemgetter(0))):
        for _, j in group:
            ret[j] = i
    return ret

def evaluate_predictors(*predictor_factories):
    results = {}
    predictors = []
    datasets = [dataset.name for dataset in open_datasets()]

    for predictor_factory in predictor_factories:
        predictor = str(predictor_factory)
        predictors.append(predictor)

        for dataset in open_datasets():
            assert len(dataset.field_types) == 1
            results[dataset.name, predictor] = evaluate_predictor_dataset(
                predictor_factory(dataset.field_types[0]), dataset
            )

    predictors.append("Gzip")
    predictors.append("Gzip diff")
    for dataset in open_datasets():
        results[dataset.name, "Gzip"] = evaluate_gzip_dataset(dataset, False)
        results[dataset.name, "Gzip diff"] = evaluate_gzip_dataset(dataset, True)

    ranking_scores = {}
    for dataset_name in datasets:
        r = ranking(
            results[dataset_name, predictor_name][0] / results[dataset_name, predictor_name][2]
            for predictor_name in predictors
        )
        for predictor_name, v in zip(predictors, r):
            if predictor_name in ranking_scores:
                ranking_scores[predictor_name] = ranking_scores[predictor_name] + v
            else:
                ranking_scores[predictor_name] = v


    table_header = ["Dataset"]
    table_header.extend(predictors)

    table_data = []
    for dataset_name in datasets:
        row = [dataset_name]
        best_predictor_length = min(
            results[dataset_name, predictor_name][0]
            for predictor_name in predictors
            if "Gzip" not in predictor_name
        )
        for predictor_name in predictors:
            sum_bits, sum_abs_error, count = results[dataset_name, predictor_name]
            #row.append(f"{sum_abs_error / count:.1f}; {sum_bits / count:.1f}b")
            percent_to_best = round(sum_bits * 100 / best_predictor_length) - 100
            to_best = "👍" if percent_to_best <= 0 else f"(+{percent_to_best}%)"
            row.append(f"{sum_bits / count:.2f}b {to_best}")
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
        #row.append(f"{total_sum_abs_error / total_count:.1f}; {total_sum_bits / total_count:.1f}b")
        row.append(f"{total_sum_bits / total_count:.2f}b")
    table_data.append(row)

    row = ["Average (equal weights)"]
    for predictor_name in predictors:
        total_sum_bits = 0
        for dataset_name in datasets:
            sum_bits, sum_abs_error, count = results[dataset_name, predictor_name]
            total_sum_bits += sum_bits / count
        #row.append(f"{total_sum_abs_error / total_count:.1f}; {total_sum_bits / total_count:.1f}b")
        row.append(f"{total_sum_bits / len(datasets):.2f}b")
    table_data.append(row)

    row = ["Ranking score"]
    for predictor_name in predictors:
        row.append(f"{ranking_scores[predictor_name]}")
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
    predictors.Last.factory(),
    predictors.Linear.factory(2),
    predictors.LinearO2.factory(),
    predictors.Linear.factory(3),
    predictors.Linear.factory(5),
    predictors.LSTSQLinear4.factory(),
    predictors.LSTSQQuadratic5.factory(),
    predictors.DoubleExponential.factory(1, 1),
    predictors.DoubleExponential.factory(1, 2),
    predictors.DoubleExponential.factory(2, 0),
    predictors.DoubleExponential.factory(2, 2),
    predictors.SmoothDeriv.factory(2),
    predictors.SmoothDeriv.factory(3),
    predictors.SmoothDeriv2.factory(2),
    predictors.SmoothDeriv2.factory(4),
    predictors.Adapt.factory(128, predictors.Last.factory(), predictors.Linear.factory(2)),
    predictors.Adapt.factory(128, predictors.Last.factory(), predictors.LinearO2.factory()),
    predictors.Adapt.factory(128, predictors.Last.factory(), predictors.Linear.factory(3)),
    predictors.Adapt.factory(128, predictors.Last.factory(), predictors.Linear.factory(5)),
    predictors.Adapt.factory(128, predictors.Last.factory(), predictors.LSTSQLinear4.factory()),
    predictors.Adapt.factory(128, predictors.Last.factory(), predictors.LSTSQQuadratic5.factory()),
)
