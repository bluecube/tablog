#include "dynamic/encoder.hpp"
#include "dynamic/predictor.hpp"
#include "dynamic/tablog.hpp"

#include "json_encoder.hpp"
#include "stat_encoder.hpp"
#include "stream_encoder.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <cstdlib>

using namespace tablog;

struct StreamOutput {
    void operator()(uint8_t out) {
        stream.put(out);
    }

    std::ostream& stream;
};

std::vector<std::pair<std::string, std::function<dynamic::Encoder(std::ostream&)>>> encoderFactories = {
    {
        "stat",
        [](std::ostream& stream) {
            return std::make_unique<test::StatEncoder>(stream);
        }
    },
    {
        "json",
        [](std::ostream& stream) {
            return std::make_unique<test::JsonEncoder>(stream);
        }
    },
    {
        "stream",
        [](std::ostream& stream) {
            return dynamic::make_dynamic_encoder<detail::StreamEncoder<StreamOutput>>(StreamOutput{stream});
        }
    }
};

std::vector<std::pair<std::string, std::function<dynamic::Predictor()>>> predictorFactories = {
    {
        "linear5",
        []() {
            return dynamic::make_dynamic_predictor<predictors::SimpleLinear<int64_t, 5>>();
        }
    }
};

void usage(const char* progName) {
    std::cout << "Usage: " << progName << "IN OUT [[ENCODER [PREDICTOR]]\n";
    std::cout << "  IN and OUT might be filenames or '-' for stdin/stdout\n";
    std::cout << "\n";

    std::cout << "ENCODER must be one of ";
    bool first = true;
    for (const auto& encoder: encoderFactories) {
        if (!first)
            std::cout << ", ";
        std::cout << encoder.first;
        first = false;
    }
    std::cout << "\nDefault encoder is " << encoderFactories.begin()->first << "\n";

    std::cout << "PREDICTOR must be one of ";
    first = true;
    for (const auto& predictor: predictorFactories) {
        if (!first)
            std::cout << ", ";
        std::cout << predictor.first;
        first = false;
    }
    std::cout << "\nDefault predictor is " << predictorFactories.begin()->first << "\n";
}

template <typename FactoryMap, typename... Args>
auto from_factory(const FactoryMap& factoryMap, const std::string& key, Args&& ...args) {
    if (key.empty())
        return factoryMap.begin()->second(std::forward<Args>(args)...);
    else {
        for (const auto& item: factoryMap) {
            if (item.first == key)
                return item.second(std::forward<Args>(args)...);
        }
        throw std::runtime_error("Key not found");
    }
}

dynamic::Tablog make_tablog(
    const std::string encoderStr,
    const std::string predictorStr,
    const std::string& labels,
    std::ostream& stream)
{
    auto encoder = from_factory(encoderFactories, encoderStr, stream);
    auto predictor = from_factory(predictorFactories, predictorStr);

    uint32_t valueCount = 1;
    for (auto c: labels)
        if (c == ',')
            ++valueCount;

    return dynamic::Tablog(std::move(encoder), *predictor, valueCount);
}

std::vector<int64_t> parse_line(const std::string& line) {
    std::vector<int64_t> ret;

    const char* p = line.data();
    char* end;

    while (true) {
        auto value = strtoll(p, &end, 10);

        if (p == end)
            throw std::runtime_error("Error when parsing data line, no number found");

        ret.push_back(value);

        if (*end == '\n' || *end == '\r' || *end == '\0')
            break;
        else if (*end == ',')
            p = end + 1;
        else
            throw std::runtime_error("Error when parsing data line, junk character found");
    }

    return ret;
}

int main(int argc, const char** argv) {
    if (argc < 3 || argc > 5) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    for (size_t i = 1; i < static_cast<size_t>(argc); ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
    }

    std::ifstream fileInput;
    auto& input = [&]() -> std::istream& {
        if (argv[1][0] == '-' && argv[1][1] == '\0')
            return std::cin;
        else {
            fileInput.open(argv[1]);
            return fileInput;
        }
    }();
    std::ofstream fileOutput;
    auto& output = [&]() -> std::ostream& {
        if (argv[2][0] == '-' && argv[2][1] == '\0')
            return std::cout;
        else {
            fileOutput.open(argv[2]);
            return fileOutput;
        }
    }();

    std::string encoderStr;
    if (argc >= 4)
        encoderStr = argv[3];
    std::string predictorStr;
    if (argc >= 5)
        predictorStr = argv[4];

    std::string columnLabels;
    std::getline(input, columnLabels);

    auto tablog = make_tablog(encoderStr, predictorStr, columnLabels, output);

    std::string lineStr;
    while(std::getline(input, lineStr))
        tablog.write(parse_line(lineStr));
}
