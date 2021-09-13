class ZeroPredictor:
    def predict(self):
        return 0

    def feed(self, value):
        pass

    def __str__(self):
        return "ZeroPredictor()"

class _HistoryPredictor:
    def __init__(self, history_length):
        self._history = [0] * history_length

    def feed(self, value):
        self._history = self._history[1:] + [value]


class SimpleLinearPredictor(_HistoryPredictor):
    def __init__(self, history_length):
        super().__init__(history_length)

    def predict(self):
        prediction = self._history[-1]
        if len(self._history) > 1:
            prediction += (self._history[-1] - self._history[0]) // (len(self._history) - 1)
        return prediction

    def __str__(self):
        return f"SimpleLinearPredictor({len(self._history)})"


class GeneralizedEWMA:
    def __init__(self, degree, smoothing):
        self._derivatives = [0] * degree
        self._smoothing = smoothing

    def predict(self):
        return sum(self._derivatives)

    def feed(self, value):
        new_derivatives = [value]
        for old_derivative in self._derivatives[:-1]:
            new_derivatives.append(new_derivatives[-1] - old_derivative)

        self._derivatives = [self._smoothing * old + (1 - self._smoothing) * new for old, new in zip(self._derivatives, self._derivatives)]

    def __str__(self):
        return f"GeneralizedEWMA({len(self._derivatives)}, {self._smoothing})"
