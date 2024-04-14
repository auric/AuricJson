#include "auric_json.h"

#include "tests.h"

#include <iostream>

#include <benchmark/benchmark.h>

int main(int argc, char** argv) {
    runTests();
    runComplexTests();
    std::cout << "All tests passed!" << std::endl;

    std::string jsonStr = R"(
        {
            "name": "John",
            "age": 30,
            "city": "New York",
            "hobbies": ["reading", "traveling"],
            "married": false,
            "score": 7.5,
            "address": null
        }
    )";

    JsonParser parser;
    try {
        auto jsonValue = parser.parse(jsonStr);
        // Access parsed values
        // ...
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    return 0;
}
