import itertools

def parse_type(t):
    """ Return closed interval of values """
    scale_bits = int(t[1:])
    if t[0] == "s":
        high = 1 << (scale_bits - 1)
        low = -high
    elif t[0] == "u":
        low = 0
        high = 1 << scale_bits
    else:
        raise ValueError("Type must be 's' or 'u' (have " + t + ")")

    return (low, high - 1)


class _Predictor:
    @classmethod
    def factory(cls, *args, **kwargs):
        ret = lambda t: cls(t, *args, **kwargs)
        ret.__name__ = cls.__name__
        if len(args) or len(kwargs):
            ret.__name__ += (
                "("
                + ", ".join(
                    itertools.chain(
                        (f"{x}" for x in args), ("{k}={v}" for k, v in kwargs.items())
                    )
                )
                + ")"
            )
        return ret


class _HistoryPredictor(_Predictor):
    def __init__(self, t, history_length):
        self._history = [0] * history_length
        (self._min, self._max) = parse_type(t)

    def feed(self, value):
        self._history = self._history[1:] + [value]


class Linear(_HistoryPredictor):
    def __init__(self, t, history_length):
        super().__init__(t, history_length)

    def predict(self):
        if len(self._history) == 1:
            return self._history[0]

        prediction = self._history[-1]
        first = self._history[0]
        last = self._history[-1]
        if first > last:
            prediction -= (first - last) // (len(self._history) - 1)
            if prediction < self._min:
                prediction = self._min
        else:
            prediction += (last - first) // (len(self._history) - 1)
            if prediction > self._max:
                prediction = self._max

        return prediction

    def __str__(self):
        return f"SimpleLinearPredictor({len(self._history)})"


class LSTSQQuadratic3(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 3)

    def predict(self):
        return self._history[0] + 3 * (self._history[2] - self._history[1])

    def __str__(self):
        return f"ThreePointQuadraticPredictor()"


class LSTSQQuadratic4(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 4)

    def predict(self):
        return (
            self._history[-1]
            + (
                3 * (self._history[0] - self._history[2])
                + 5 * (self._history[3] - self._history[1])
            )
            // 4
        )

    def __str__(self):
        return f"FourPointQuadraticPredictor()"


class LSTSQQuadratic5(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 5)

    def predict(self):
        return (
            self._history[-1]
            + (
                3 * (self._history[0] - self._history[1])
                + 4 * (self._history[4] - self._history[2])
            )
            // 5
        )

    def __str__(self):
        return f"FivePointQuadraticPredictor()"


class GeneralizedEWMA(_Predictor):
    def __init__(self, t, degree, smoothing):
        self._derivatives = [0] * degree
        self._smoothing = smoothing

    def predict(self):
        return sum(self._derivatives)

    def feed(self, value):
        new_derivatives = [value]
        for old_derivative in self._derivatives[:-1]:
            new_derivatives.append(new_derivatives[-1] - old_derivative)

        self._derivatives = [
            self._smoothing * old + (1 - self._smoothing) * new
            for old, new in zip(self._derivatives, self._derivatives)
        ]

    def __str__(self):
        return f"GeneralizedEWMA({len(self._derivatives)}, {self._smoothing})"


class SmoothDeriv(_HistoryPredictor):
    def __init__(self, t, smoothing_power):
        super().__init__(t, 1)
        self._derivative = 0
        self._smoothing = smoothing_power

    def _rhs(self, v):
        if v < 0:
            return -((-v) >> self._smoothing)
        else:
            return v >> self._smoothing

    def predict(self):
        return self._history[0] + self._rhs(self._derivative)

    def feed(self, value):
        new_derivative = value - self._history[0]
        self._derivative += (new_derivative - self._rhs(self._derivative))
        super().feed(value)


class SmoothDeriv2(_HistoryPredictor):
    def __init__(self, t, smoothing_power):
        super().__init__(t, 2)
        self._second_derivative = 0
        self._smoothing = smoothing_power

    def predict(self):
        return 2 * self._history[1] - self._history[0] + self._second_derivative

    def feed(self, value):
        new_second_derivative = value - 2 * self._history[1] + self._history[0]
        self._second_derivative += (new_second_derivative - self._second_derivative) >> self._smoothing
        super().feed(value)
