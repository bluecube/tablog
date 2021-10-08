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

    def predict_and_feed(self, value):
        ret = self.predict()
        self.feed(value)
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


class DoubleExponential(_Predictor):
    def __init__(self, t, smoothing1, smoothing2):
        self._smoothing1 = smoothing1
        self._smoothing2 = smoothing2
        self._v = 0
        self._d = 0

    def predict(self):
        return self._v + self._d

    def feed(self, value):
        #self._v = s1 * value + (1 - s1) * (self._v + self._d)
        #self._d = s2 * (self._v - old_v) + (1 - s2) * self_d

        v_change = self._d + (value - (self._v + self._d)) >> self._smoothing1
        self._v += v_change
        self._d += (v_change - self._d) >> self._smoothing2

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


class Linear12Adapt(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 2)
        self._selector = 0 # >= 0 -> predict zero derivative, < 0 -> predict constant derivative
        self._selector_max = 128

    def predict_and_feed(self, new_value):

        if self._history[0] == self._history[1]:
            prediction = self._history[1]
        else:
            prediction1 = self._history[1]
            prediction2_change = self._history[1] - self._history[0]
            prediction2 = max(self._min, min(prediction1 + prediction2_change, self._max))

            if self._selector >= 0:
                prediction = prediction1
            else:
                prediction = prediction2

            error1 = prediction1 - new_value
            error2 = prediction2 - new_value

            if abs(error1) < abs(error2):
                self._selector = min(self._selector + 1, self._selector_max - 1)
            else:
                self._selector = max(self._selector - 1, -self._selector_max)

        super().feed(new_value)

        return prediction
