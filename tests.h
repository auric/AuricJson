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
        assert(std::holds_alternative<std::string>(jsonValue));
        const auto str = std::get<std::string>(jsonValue);
        assert(str == "Hello, world!");
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

    {
        std::string_view jsonStr = R"("Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:\u2728")";

        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        assert(JsonParser::isString(jsonValue));

        const auto str = JsonParser::toString(jsonValue);
        std::string expected = "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨";
        assert(str == expected);
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
        assert(std::holds_alternative<std::string>(obj.members[0].second));
        assert(std::get<std::string>(obj.members[0].second) == "John");
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
        assert(std::holds_alternative<std::string>(obj.members[0].second));
        assert(std::get<std::string>(obj.members[0].second) == "John");

        assert(obj.members[1].first == "age");
        assert(std::holds_alternative<int>(obj.members[1].second));
        assert(std::get<int>(obj.members[1].second) == 30);

        assert(obj.members[2].first == "address");
        assert(std::holds_alternative<JsonParser::JsonObject>(obj.members[2].second));
        const JsonParser::JsonObject& address = std::get<JsonParser::JsonObject>(obj.members[2].second);
        assert(address.members.size() == 2);
        assert(address.members[0].first == "street");
        assert(std::holds_alternative<std::string>(address.members[0].second));
        assert(std::get<std::string>(address.members[0].second) == "123 Main St");
        assert(address.members[1].first == "city");
        assert(std::holds_alternative<std::string>(address.members[1].second));
        assert(std::get<std::string>(address.members[1].second) == "New York");

        assert(obj.members[3].first == "hobbies");
        assert(std::holds_alternative<JsonParser::JsonArray>(obj.members[3].second));
        const JsonParser::JsonArray& hobbies = std::get<JsonParser::JsonArray>(obj.members[3].second);
        assert(hobbies.elements.size() == 2);
        assert(std::holds_alternative<std::string>(hobbies.elements[0]));
        assert(std::get<std::string>(hobbies.elements[0]) == "reading");
        assert(std::holds_alternative<std::string>(hobbies.elements[1]));
        assert(std::get<std::string>(hobbies.elements[1]) == "traveling");

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

void runComplexTests() {
    std::string_view jsonStr = R"(
        {
            "name": "John Doe",
            "age": 30,
            "height": 1.75,
            "married": false,
            "hobbies": null,
            "address": {
                "street": "123 Main St",
                "city": "New York",
                "country": "USA",
                "zipcode": "10001"
            },
            "phoneNumbers": [
                {
                    "type": "home",
                    "number": "+1-555-123-4567"
                },
                {
                    "type": "work",
                    "number": "+1-555-987-6543"
                }
            ],
            "friends": [
                {
                    "name": "Alice",
                    "age": 28,
                    "hobbies": ["reading", "painting"]
                },
                {
                    "name": "Bob",
                    "age": 32,
                    "hobbies": ["gaming", "traveling"]
                }
            ],
            "scores": [7.5, 8.2, 9.0, -3.14, 2.71828],
            "description": "Hello, world! 😊 これは日本語のテキストです。 🇯🇵",
            "escape": "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:\u2728",
            "url": "https://example.com?q=test&limit=10",
            "nested": {
                "arr": [1, [2, [3, [4, [5]]]]],
                "obj": {
                    "a": {
                        "b": {
                            "c": {
                                "d": {
                                    "e": "nested"
                                }
                            }
                        }
                    }
                }
            }
        }
    )";

    JsonParser parser;
    JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
    assert(JsonParser::isObject(jsonValue));
    const JsonParser::JsonObject& obj = JsonParser::toObject(jsonValue);

    assert(obj["name"] == "John Doe");
    assert(JsonParser::isInt(obj["age"]));
    assert(JsonParser::toInt(obj["age"]) == 30);
    assert(JsonParser::isDouble(obj["height"]));
    assert(std::abs(JsonParser::toDouble(obj["height"]) - 1.75) < 0.001);
    assert(JsonParser::isBool(obj["married"]));
    assert(JsonParser::toBool(obj["married"]) == false);
    assert(JsonParser::isNull(obj["hobbies"]));

    assert(JsonParser::isObject(obj["address"]));
    const JsonParser::JsonObject& address = JsonParser::toObject(obj["address"]);
    assert(address["street"] == "123 Main St");
    assert(address["city"] == "New York");
    assert(address["country"] == "USA");
    assert(address["zipcode"] == "10001");

    assert(JsonParser::isArray(obj["phoneNumbers"]));
    const JsonParser::JsonArray& phoneNumbers = JsonParser::toArray(obj["phoneNumbers"]);
    assert(phoneNumbers.elements.size() == 2);
    assert(JsonParser::isObject(phoneNumbers.elements[0]));
    const JsonParser::JsonObject& phoneNumber1 = JsonParser::toObject(phoneNumbers.elements[0]);
    assert(phoneNumber1["type"] == "home");
    assert(phoneNumber1["number"] == "+1-555-123-4567");

    assert(JsonParser::isArray(obj["friends"]));
    const JsonParser::JsonArray& friends = JsonParser::toArray(obj["friends"]);
    assert(friends.elements.size() == 2);
    assert(JsonParser::isObject(friends.elements[0]));
    const JsonParser::JsonObject& friend1 = JsonParser::toObject(friends.elements[0]);
    assert(friend1["name"] == "Alice");
    assert(JsonParser::isInt(friend1["age"]));
    assert(JsonParser::toInt(friend1["age"]) == 28);
    assert(JsonParser::isArray(friend1["hobbies"]));
    const JsonParser::JsonArray& hobbies1 = JsonParser::toArray(friend1["hobbies"]);
    assert(hobbies1.elements.size() == 2);
    assert(hobbies1.elements[0] == "reading");
    assert(hobbies1.elements[1] == "painting");

    assert(JsonParser::isArray(obj["scores"]));
    const JsonParser::JsonArray& scores = JsonParser::toArray(obj["scores"]);
    assert(scores.elements.size() == 5);
    assert(JsonParser::isDouble(scores.elements[0]));
    assert(std::abs(JsonParser::toDouble(scores.elements[0]) - 7.5) < 0.001);
    assert(JsonParser::isDouble(scores.elements[1]));
    assert(std::abs(JsonParser::toDouble(scores.elements[1]) - 8.2) < 0.001);
    assert(JsonParser::isDouble(scores.elements[2]));
    assert(std::abs(JsonParser::toDouble(scores.elements[2]) - 9.0) < 0.001);
    assert(JsonParser::isDouble(scores.elements[3]));
    assert(std::abs(JsonParser::toDouble(scores.elements[3]) - (-3.14)) < 0.001);
    assert(JsonParser::isDouble(scores.elements[4]));
    assert(std::abs(JsonParser::toDouble(scores.elements[4]) - 2.71828) < 0.001);

    assert(obj["description"] == "Hello, world! 😊 これは日本語のテキストです。 🇯🇵");
    const auto escaped = obj["escape"];
    assert(escaped == "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨");
    assert(obj["url"] == "https://example.com?q=test&limit=10");

    assert(JsonParser::isObject(obj["nested"]));
    const JsonParser::JsonObject& nested = JsonParser::toObject(obj["nested"]);
    assert(JsonParser::isArray(nested["arr"]));
    const JsonParser::JsonArray& nestedArr = JsonParser::toArray(nested["arr"]);
    assert(nestedArr.elements.size() == 2);
    assert(JsonParser::isInt(nestedArr.elements[0]));
    assert(JsonParser::toInt(nestedArr.elements[0]) == 1);
    assert(JsonParser::isArray(nestedArr.elements[1]));
    const JsonParser::JsonArray& nestedArr2 = JsonParser::toArray(nestedArr.elements[1]);
    assert(nestedArr2.elements.size() == 2);
    assert(JsonParser::isInt(nestedArr2.elements[0]));
    assert(JsonParser::toInt(nestedArr2.elements[0]) == 2);
    // ... (continue assertions for nested arrays)

    assert(JsonParser::isObject(nested["obj"]));
    const JsonParser::JsonObject& nestedObj = JsonParser::toObject(nested["obj"]);
    assert(JsonParser::isObject(nestedObj["a"]));
    const JsonParser::JsonObject& nestedObjA = JsonParser::toObject(nestedObj["a"]);
    assert(JsonParser::isObject(nestedObjA["b"]));
    const JsonParser::JsonObject& nestedObjB = JsonParser::toObject(nestedObjA["b"]);
    assert(JsonParser::isObject(nestedObjB["c"]));
    const JsonParser::JsonObject& nestedObjC = JsonParser::toObject(nestedObjB["c"]);
    assert(JsonParser::isObject(nestedObjC["d"]));
    const JsonParser::JsonObject& nestedObjD = JsonParser::toObject(nestedObjC["d"]);
    assert(nestedObjD["e"] == "nested");
}
