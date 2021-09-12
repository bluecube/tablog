class SimpleLinearPredictor:
    def __init__(self, history_length):
        self._history = []
        self._history_length = history_length

    def predict(self):
        if len(self._history) == 0:
            return 0

        prediction = self._history[-1]
        if len(self._history) > 1:
            prediction += (self._history[-1] - self._history[0]) // (len(self._history) - 1)

        return prediction

    def feed(self, value):
        if len(self._history) < self._history_length:
            self._history.append(value)
        else:
            self._history = self._history[1:] + [value]

    def __str__(self):
        return f"SimpleLinearPredictor({self._history_length})"


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
