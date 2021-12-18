#include "../common.hpp"

#include "tablog.hpp"

#include <string_view>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <stdexcept>

std::string_view trim_whitespace(std::string_view s) {
    constexpr std::string_view whitespace = " 	\r\n";

    const auto start = s.find_first_not_of(whitespace);

    if (start == std::string_view::npos)
        return s.substr(0, 0);

    const auto end = s.find_last_not_of(whitespace);
    return s.substr(start, end - start + 1);
}

std::vector<std::string_view> tokenize_csv_line(std::string_view line) {

    std::vector<std::string_view> ret;

    while (const auto token = next_token(line))
        ret.push_back(trim_whitespace(*token));

    return ret;
}

struct StreamOutput {
    void operator()(uint8_t out) {
        stream.put(out);
    }

    std::ostream& stream;
};

class CsvRowEncoderInterface
{
public:
    virtual ~CsvRowEncoderInterface() {}
    virtual void encode(const std::vector<std::string_view>& row) = 0;
};

template <typename... ValueTs>
class TablogCsvRowEncoder : public CsvRowEncoderInterface {
public:
    template <typename... EncoderArgs>
    TablogCsvRowEncoder(StreamOutput output, const std::vector<std::string_view>& columnNames)
      : t(std::move(output)) {
        (void)columnNames;
    }

    void encode(const std::vector<std::string_view>& row) override {
        if (row.size() != sizeof...(ValueTs))
            throw std::runtime_error("Wrong number of records on row");

        encode(row, std::index_sequence_for<ValueTs...>{});
    }

protected:
    template <std::size_t... Indices>
    void encode(const std::vector<std::string_view>& row, std::index_sequence<Indices...>) {
        t.write(parse_num<ValueTs>(row[Indices])...);
    }


    tablog::Tablog<StreamOutput, ValueTs...> t;
};


#define FACTORY_ENTRY(types...) \
    { \
        std::vector<std::string_view>(type_names<types>.begin(), type_names<types>.end()), \
        [](auto output, const auto& columnNames) { \
            return std::make_unique<TablogCsvRowEncoder<types>>(std::move(output), columnNames); \
        } \
    }

const std::map<
    std::vector<std::string_view>, // Value types
    std::function<
        std::unique_ptr<CsvRowEncoderInterface>(StreamOutput output, const std::vector<std::string_view>&)
    > // Factory function
> factories = {
    FACTORY_ENTRY(int8_t),
    FACTORY_ENTRY(uint8_t),
    FACTORY_ENTRY(int16_t),
    FACTORY_ENTRY(uint16_t),
    FACTORY_ENTRY(int32_t),
    FACTORY_ENTRY(uint32_t),
    FACTORY_ENTRY(int64_t),
    FACTORY_ENTRY(uint64_t),
    FACTORY_ENTRY(int64_t, uint32_t, uint16_t, uint16_t, uint16_t, uint32_t) // dataset1/tph.csv
};


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

int main() {
    std::string columnLabelsStr;
    std::getline(std::cin, columnLabelsStr);
    const auto columnLabels = tokenize_csv_line(columnLabelsStr);
    std::string typesStr;
    std::getline(std::cin, typesStr);
    const auto types = tokenize_csv_line(typesStr);

    const auto factoryIt = factories.find(types);
    if (factoryIt == factories.end())
        return 128;

    auto tablogInterface = factoryIt->second(StreamOutput{std::cout}, columnLabels);

    std::string rowStr;
    while(std::getline(std::cin, rowStr)) {
        const auto row = tokenize_csv_line(rowStr);
        tablogInterface->encode(row);
    }
}
