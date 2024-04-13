// tests.cpp
#include <cassert>
#include <cmath>
#include <string_view>
#include "auric_json.h"

void runTests() {
    // Test parsing null
    {
        std::string_view jsonStr = "null";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<std::nullptr_t>(jsonValue));
    }

    // Test parsing boolean values
    {
        std::string_view jsonStr = "true";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<bool>(jsonValue));
        assert(std::get<bool>(jsonValue) == true);
    }

    {
        std::string_view jsonStr = "false";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<bool>(jsonValue));
        assert(std::get<bool>(jsonValue) == false);
    }

    // Test parsing numbers
    {
        std::string_view jsonStr = "42";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<int>(jsonValue));
        assert(std::get<int>(jsonValue) == 42);
    }

    {
        std::string_view jsonStr = "-3.14";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<double>(jsonValue));
        assert(std::abs(std::get<double>(jsonValue) - (-3.14)) < 0.001);
    }

    // Test parsing strings
    {
        std::string_view jsonStr = R"("Hello, world!")";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<std::string_view>(jsonValue));
        assert(std::get<std::string_view>(jsonValue) == "Hello, world!");
    }

    // Test parsing Unicode characters
    {
        std::string_view jsonStr = R"("Hello, 世界!")";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(JsonParser::isString(jsonValue));
        assert(JsonParser::toString(jsonValue) == "Hello, 世界!");
    }

    {
        std::string_view jsonStr = R"("Émoji 😃 Привіт")";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(JsonParser::isString(jsonValue));
        assert(JsonParser::toString(jsonValue) == "Émoji 😃 Привіт");
    }

    {
        std::string_view jsonStr = R"("Escape sequences: \"\\/\b\f\n\r\t\u2028\u2029")";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(JsonParser::isString(jsonValue));

        std::string expectedStr = "Escape sequences: \"\\/\b\f\n\r\t";
        expectedStr += static_cast<char>(0xE2);
        expectedStr += static_cast<char>(0x80);
        expectedStr += static_cast<char>(0xA8);
        expectedStr += static_cast<char>(0xE2);
        expectedStr += static_cast<char>(0x80);
        expectedStr += static_cast<char>(0xA9);

        assert(JsonParser::toString(jsonValue) == expectedStr);
    }

    // Test parsing arrays
    {
        std::string_view jsonStr = R"([1, 2, 3])";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<JsonParser::JsonArray>(jsonValue));
        const JsonParser::JsonArray& arr = std::get<JsonParser::JsonArray>(jsonValue);
        assert(arr.elements.size() == 3);
        assert(std::holds_alternative<int>(arr.elements[0]));
        assert(std::get<int>(arr.elements[0]) == 1);
        assert(std::holds_alternative<int>(arr.elements[1]));
        assert(std::get<int>(arr.elements[1]) == 2);
        assert(std::holds_alternative<int>(arr.elements[2]));
        assert(std::get<int>(arr.elements[2]) == 3);
    }

    // Test parsing objects
    {
        std::string_view jsonStr = R"({"name": "John", "age": 30})";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<JsonParser::JsonObject>(jsonValue));
        const JsonParser::JsonObject& obj = std::get<JsonParser::JsonObject>(jsonValue);
        assert(obj.members.size() == 2);
        assert(obj.members[0].first == "name");
        assert(std::holds_alternative<std::string_view>(obj.members[0].second));
        assert(std::get<std::string_view>(obj.members[0].second) == "John");
        assert(obj.members[1].first == "age");
        assert(std::holds_alternative<int>(obj.members[1].second));
        assert(std::get<int>(obj.members[1].second) == 30);
    }

    // Test parsing nested structures
    {
        std::string_view jsonStr = R"(
            {
                "name": "John",
                "age": 30,
                "address": {
                    "street": "123 Main St",
                    "city": "New York"
                },
                "hobbies": ["reading", "traveling"],
                "scores": [7.5, 8.2, 9.0],
                "employed": true
            }
        )";
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(std::holds_alternative<JsonParser::JsonObject>(jsonValue));
        const JsonParser::JsonObject& obj = std::get<JsonParser::JsonObject>(jsonValue);
        assert(obj.members.size() == 6);

        assert(obj.members[0].first == "name");
        assert(std::holds_alternative<std::string_view>(obj.members[0].second));
        assert(std::get<std::string_view>(obj.members[0].second) == "John");

        assert(obj.members[1].first == "age");
        assert(std::holds_alternative<int>(obj.members[1].second));
        assert(std::get<int>(obj.members[1].second) == 30);

        assert(obj.members[2].first == "address");
        assert(std::holds_alternative<JsonParser::JsonObject>(obj.members[2].second));
        const JsonParser::JsonObject& address = std::get<JsonParser::JsonObject>(obj.members[2].second);
        assert(address.members.size() == 2);
        assert(address.members[0].first == "street");
        assert(std::holds_alternative<std::string_view>(address.members[0].second));
        assert(std::get<std::string_view>(address.members[0].second) == "123 Main St");
        assert(address.members[1].first == "city");
        assert(std::holds_alternative<std::string_view>(address.members[1].second));
        assert(std::get<std::string_view>(address.members[1].second) == "New York");

        assert(obj.members[3].first == "hobbies");
        assert(std::holds_alternative<JsonParser::JsonArray>(obj.members[3].second));
        const JsonParser::JsonArray& hobbies = std::get<JsonParser::JsonArray>(obj.members[3].second);
        assert(hobbies.elements.size() == 2);
        assert(std::holds_alternative<std::string_view>(hobbies.elements[0]));
        assert(std::get<std::string_view>(hobbies.elements[0]) == "reading");
        assert(std::holds_alternative<std::string_view>(hobbies.elements[1]));
        assert(std::get<std::string_view>(hobbies.elements[1]) == "traveling");

        assert(obj.members[4].first == "scores");
        assert(std::holds_alternative<JsonParser::JsonArray>(obj.members[4].second));
        const JsonParser::JsonArray& scores = std::get<JsonParser::JsonArray>(obj.members[4].second);
        assert(scores.elements.size() == 3);
        assert(std::holds_alternative<double>(scores.elements[0]));
        assert(std::abs(std::get<double>(scores.elements[0]) - 7.5) < 0.001);
        assert(std::holds_alternative<double>(scores.elements[1]));
        assert(std::abs(std::get<double>(scores.elements[1]) - 8.2) < 0.001);
        assert(std::holds_alternative<double>(scores.elements[2]));
        assert(std::abs(std::get<double>(scores.elements[2]) - 9.0) < 0.001);

        assert(obj.members[5].first == "employed");
        assert(std::holds_alternative<bool>(obj.members[5].second));
        assert(std::get<bool>(obj.members[5].second) == true);
    }
}
