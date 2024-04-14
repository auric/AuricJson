#include "auric_json.h"

#include <iostream>

int main(int argc, char** argv) {
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

    return 0;
}
