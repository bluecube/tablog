#pragma once

#include <cstdint>
#include <memory>
#include <limits>
#include <stdexcept>
#include <type_traits>

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

    /// Return signedness of the predictor type.
    virtual bool t_is_signed() const = 0;

    /// Return size of the predictor type
    virtual std::size_t sizeof_t() const = 0;

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
        using Type = typename T::Type;
        if (value > std::numeric_limits<Type>::max() ||
            value < std::numeric_limits<Type>::min())
            throw std::runtime_error("Predictor value out of range");
        predictor.feed(static_cast<Type>(value));
    }

    /// Return signedness of the predictor type.
    bool t_is_signed() const override {
        return std::is_signed_v<typename T::Type>;
    }

    /// Return size of the predictor type
    std::size_t sizeof_t() const override {
        return sizeof(typename T::Type);
    }

    Predictor make_new() const override {
        return make_dynamic_predictor<T>();
    }
private:
    T predictor;
};

}
