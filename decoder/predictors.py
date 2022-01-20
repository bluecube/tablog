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
