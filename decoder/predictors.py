import itertools


class _PredictorFactory:
    def __init__(self, cls, args, kwargs):
        self._cls = cls
        self._args = args
        self._kwargs = kwargs

    def __call__(self, t):
        return self._cls(t, *self._args, **self._kwargs)

    def __str__(self):
        return (
            self._cls.__name__
            + "("
            + ", ".join(
                itertools.chain(
                    (f"{x}" for x in self._args),
                    ("{k}={v}" for k, v in self._kwargs.items()),
                )
            )
            + ")"
        )


class _Predictor:
    @classmethod
    def factory(cls, *args, **kwargs):
        return _PredictorFactory(cls, args, kwargs)

    def predict_and_feed(self, value):
        ret = self.predict()
        self.feed(value)
        return ret


class _HistoryPredictor(_Predictor):
    def __init__(self, t, history_length):
        self._history = [0] * history_length
        (self._min, self._max) = t.minmax()

    def feed(self, value):
        self._history = self._history[1:] + [value]


class Last(_Predictor):
    def __init__(self, _):
        self._last = 0

    def feed(self, value):
        self._last = value

    def predict(self):
        return self._last


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


class LinearO2(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 2)
        self._range = self._max - self._min + 1

    def predict(self):
        prediction = 2 * self._history[-1] - self._history[0]

        if prediction > self._max:
            prediction -= self._range
        elif prediction < self._min:
            prediction += self._range

        return prediction


class LSTSQLinear4(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 4)

    def predict(self):
        return self._history[3] + (self._history[2] - self._history[0]) / 2


class LSTSQQuadratic3(_HistoryPredictor):
    def __init__(self, t):
        super().__init__(t, 3)

    def predict(self):
        return self._history[0] + 3 * (self._history[2] - self._history[1])


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


class DoubleExponential(_Predictor):
    def __init__(self, t, smoothing1, smoothing2):
        self._smoothing1 = smoothing1
        self._smoothing2 = smoothing2
        self._v = 0
        self._d = 0

    def predict(self):
        return self._v + self._d

    def feed(self, value):
        # self._v = s1 * value + (1 - s1) * (self._v + self._d)
        # self._d = s2 * (self._v - old_v) + (1 - s2) * self_d

        v_change = self._d + (value - (self._v + self._d)) >> self._smoothing1
        self._v += v_change
        self._d += (v_change - self._d) >> self._smoothing2


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
        self._derivative += new_derivative - self._rhs(self._derivative)
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
        self._second_derivative += (
            new_second_derivative - self._second_derivative
        ) >> self._smoothing
        super().feed(value)


class Adapt(_Predictor):
    def __init__(self, t, selector_max, factory1, factory2):
        super().__init__()
        self._selector = 0  # >= 0 -> p1, < 0 -> p2
        self._selector_max = selector_max
        self._p1 = factory1(t)
        self._p2 = factory2(t)

    def predict(self):
        if self._selector >= 0:
            return self._p1.predict()
        else:
            return self._p2.predict()

    def feed(self, new_value):
        prediction1 = self._p1.predict()
        prediction2 = self._p2.predict()

        error1 = abs(prediction1 - new_value)
        error2 = abs(prediction2 - new_value)

        if error1 < error2:
            self._selector = min(self._selector + 1, self._selector_max - 1)
        elif error1 > error2:
            self._selector = max(self._selector - 1, -self._selector_max)

        self._p1.feed(new_value)
        self._p2.feed(new_value)
