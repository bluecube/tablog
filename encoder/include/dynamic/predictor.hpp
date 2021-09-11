#pragma once

#include <cstdint>
#include <memory>

namespace tablog::dynamic {

class PredictorInterface;
template <typename T> class PredictorAdapter;

using Predictor = std::unique_ptr<PredictorInterface>;
template <typename T, typename... Args>
Predictor make_dynamic_predictor(Args&&... args) {
    return std::make_unique<PredictorAdapter<T>>(std::forward<Args>(args)...);
}

class PredictorInterface {
public:
    virtual ~PredictorInterface() {}

    /// Returns a predicted value
    /// Prediction always works, regardless of number of data points provided.
    virtual int64_t predict() const = 0;

    /// Feed a new value to the predictor
    virtual void feed(int64_t value) = 0;

    /// Create a new default constructed instance of the predictor
    virtual Predictor make_new() const = 0;
};

template <typename T>
class PredictorAdapter: public PredictorInterface {
public:
    template <typename... Args>
    PredictorAdapter(Args&&... args) : predictor(std::forward<Args>(args)...) {}

    int64_t predict() const override {
        return predictor.predict();
    }

    void feed(int64_t value) override {
        predictor.feed(value);
    }

    Predictor make_new() const override {
        return make_dynamic_predictor<T>();
    }
private:
    T predictor;
};

}
