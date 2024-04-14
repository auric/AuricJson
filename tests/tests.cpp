// tests.cpp
#include <gtest/gtest.h>
#include <limits>
#include "../auric_json.h"

using namespace std::string_view_literals;

TEST(JsonParser, ParseNull) {
    std::string_view jsonStr = "null"sv;
    JsonParser parser;
    JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(jsonValue));
}

TEST(JsonParser, ParseBooleans) {
    {
        std::string_view jsonStr = "true"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<bool>(jsonValue));
        EXPECT_EQ(std::get<bool>(jsonValue), true);
    }

    {
        std::string_view jsonStr = "false"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<bool>(jsonValue));
        EXPECT_EQ(std::get<bool>(jsonValue), false);
    }
}

TEST(JsonParser, ParseNumbers) {
    {
        std::string_view jsonStr = "42"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<int>(jsonValue));
        EXPECT_EQ(std::get<int>(jsonValue), 42);
    }

    {
        std::string_view jsonStr = "-3.14"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<double>(jsonValue));
        EXPECT_NEAR(std::get<double>(jsonValue), -3.14, 0.001);
    }
}

TEST(JsonParser, ParseStrings) {
    {
        std::string_view jsonStr = R"("Hello, world!")"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<std::string>(jsonValue));
        EXPECT_EQ(std::get<std::string>(jsonValue), "Hello, world!");
    }
}

TEST(JsonParser, ParseUnicodeStrings) {
    {
        std::string_view jsonStr = R"("Hello, 世界!")"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isString(jsonValue));
        EXPECT_EQ(JsonParser::toString(jsonValue), "Hello, 世界!");
    }

    {
        std::string_view jsonStr = R"("Émoji 😃 Привіт")"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isString(jsonValue));
        EXPECT_EQ(JsonParser::toString(jsonValue), "Émoji 😃 Привіт");
    }

    {
        std::string_view jsonStr = R"("Escape sequences: \"\\/\b\f\n\r\t\u2028\u2029")"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isString(jsonValue));

        std::string expectedStr = "Escape sequences: \"\\/\b\f\n\r\t";
        expectedStr += static_cast<char>(0xE2);
        expectedStr += static_cast<char>(0x80);
        expectedStr += static_cast<char>(0xA8);
        expectedStr += static_cast<char>(0xE2);
        expectedStr += static_cast<char>(0x80);
        expectedStr += static_cast<char>(0xA9);

        EXPECT_EQ(JsonParser::toString(jsonValue), expectedStr);
    }

    {
        std::string_view jsonStr = R"("Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:\u2728")"sv;

        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isString(jsonValue));

        const auto str = JsonParser::toString(jsonValue);
        std::string expected = "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨";
        EXPECT_EQ(str, expected);
    }
}

TEST(JsonParser, ParseArrays) {
    {
        std::string_view jsonStr = R"([1, 2, 3])"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<JsonParser::JsonArray>(jsonValue));
        const JsonParser::JsonArray& arr = std::get<JsonParser::JsonArray>(jsonValue);
        EXPECT_EQ(arr.elements.size(), 3U);
        EXPECT_TRUE(std::holds_alternative<int>(arr.elements[0]));
        EXPECT_EQ(std::get<int>(arr.elements[0]), 1);
        EXPECT_TRUE(std::holds_alternative<int>(arr.elements[1]));
        EXPECT_EQ(std::get<int>(arr.elements[1]), 2);
        EXPECT_TRUE(std::holds_alternative<int>(arr.elements[2]));
        EXPECT_EQ(std::get<int>(arr.elements[2]), 3);
    }
}

TEST(JsonParser, ParseObjects) {
    {
        std::string_view jsonStr = R"({"name": "John", "age": 30})"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<JsonParser::JsonObject>(jsonValue));
        const JsonParser::JsonObject& obj = std::get<JsonParser::JsonObject>(jsonValue);
        EXPECT_EQ(obj.members.size(), 2U);
        EXPECT_EQ(obj.members[0].first, "name");
        EXPECT_TRUE(std::holds_alternative<std::string>(obj.members[0].second));
        EXPECT_EQ(std::get<std::string>(obj.members[0].second), "John");
        EXPECT_EQ(obj.members[1].first, "age");
        EXPECT_TRUE(std::holds_alternative<int>(obj.members[1].second));
        EXPECT_EQ(std::get<int>(obj.members[1].second), 30);
    }
}

TEST(JsonParser, ParseNestedStructures) {
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
    )"sv;

    JsonParser parser;
    JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(std::holds_alternative<JsonParser::JsonObject>(jsonValue));
    const JsonParser::JsonObject& obj = std::get<JsonParser::JsonObject>(jsonValue);
    EXPECT_EQ(obj.members.size(), 6U);

    EXPECT_EQ(obj.members[0].first, "name");
    EXPECT_TRUE(std::holds_alternative<std::string>(obj.members[0].second));
    EXPECT_EQ(std::get<std::string>(obj.members[0].second), "John");

    EXPECT_EQ(obj.members[1].first, "age");
    EXPECT_TRUE(std::holds_alternative<int>(obj.members[1].second));
    EXPECT_EQ(std::get<int>(obj.members[1].second), 30);

    EXPECT_EQ(obj.members[2].first, "address");
    EXPECT_TRUE(std::holds_alternative<JsonParser::JsonObject>(obj.members[2].second));
    const JsonParser::JsonObject& address = std::get<JsonParser::JsonObject>(obj.members[2].second);
    EXPECT_EQ(address.members.size(), 2U);
    EXPECT_EQ(address.members[0].first, "street");
    EXPECT_TRUE(std::holds_alternative<std::string>(address.members[0].second));
    EXPECT_EQ(std::get<std::string>(address.members[0].second), "123 Main St");
    EXPECT_EQ(address.members[1].first, "city");
    EXPECT_TRUE(std::holds_alternative<std::string>(address.members[1].second));
    EXPECT_EQ(std::get<std::string>(address.members[1].second), "New York");

    EXPECT_EQ(obj.members[3].first, "hobbies");
    EXPECT_TRUE(std::holds_alternative<JsonParser::JsonArray>(obj.members[3].second));
    const JsonParser::JsonArray& hobbies = std::get<JsonParser::JsonArray>(obj.members[3].second);
    EXPECT_EQ(hobbies.elements.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<std::string>(hobbies.elements[0]));
    EXPECT_EQ(std::get<std::string>(hobbies.elements[0]), "reading");
    EXPECT_TRUE(std::holds_alternative<std::string>(hobbies.elements[1]));
    EXPECT_EQ(std::get<std::string>(hobbies.elements[1]), "traveling");

    EXPECT_EQ(obj.members[4].first, "scores");
    EXPECT_TRUE(std::holds_alternative<JsonParser::JsonArray>(obj.members[4].second));
    const JsonParser::JsonArray& scores = std::get<JsonParser::JsonArray>(obj.members[4].second);
    EXPECT_EQ(scores.elements.size(), 3U);
    EXPECT_TRUE(std::holds_alternative<double>(scores.elements[0]));
    EXPECT_NEAR(std::get<double>(scores.elements[0]), 7.5, 0.001);
    EXPECT_TRUE(std::holds_alternative<double>(scores.elements[1]));
    EXPECT_NEAR(std::get<double>(scores.elements[1]), 8.2, 0.001);
    EXPECT_TRUE(std::holds_alternative<double>(scores.elements[2]));
    EXPECT_NEAR(std::get<double>(scores.elements[2]), 9.0, 0.001);

    EXPECT_EQ(obj.members[5].first, "employed");
    EXPECT_TRUE(std::holds_alternative<bool>(obj.members[5].second));
    EXPECT_EQ(std::get<bool>(obj.members[5].second), true);
}

TEST(JsonParser, ParseEmptyArraysAndObjects) {
    {
        std::string_view jsonStr = "[]"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isArray(jsonValue));
        const auto& arr = JsonParser::toArray(jsonValue);
        EXPECT_TRUE(arr.elements.empty());
    }

    {
        std::string_view jsonStr = "{}"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isObject(jsonValue));
        const auto& obj = JsonParser::toObject(jsonValue);
        EXPECT_TRUE(obj.members.empty());
    }
}

TEST(JsonParser, ParseNumbersWithExponents) {
    {
        std::string_view jsonStr = "1.23e+4"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isDouble(jsonValue));
        EXPECT_NEAR(JsonParser::toDouble(jsonValue), 1.23e+4, 0.001);
    }

    {
        std::string_view jsonStr = "-5.67E-8"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isDouble(jsonValue));
        EXPECT_NEAR(JsonParser::toDouble(jsonValue), -5.67e-8, 0.001);
    }
}

TEST(JsonParser, ParseNumbersWithLeadingZeros) {
    {
        std::string_view jsonStr = "0123"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isInt(jsonValue));
        EXPECT_EQ(JsonParser::toInt(jsonValue), 123);
    }
}

TEST(JsonParser, ParseArrayWithTrailingComma) {
    {
        std::string_view jsonStr = "[1, 2, 3,]"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isArray(jsonValue));
        const auto& arr = JsonParser::toArray(jsonValue);
        EXPECT_EQ(arr.elements.size(), 3U);
        EXPECT_EQ(JsonParser::toInt(arr.elements[0]), 1);
        EXPECT_EQ(JsonParser::toInt(arr.elements[1]), 2);
        EXPECT_EQ(JsonParser::toInt(arr.elements[2]), 3);
    }
}

TEST(JsonParser, ParseObjectWithTrailingComma) {
    {
        std::string_view jsonStr = R"({"a": 1, "b": 2,})"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isObject(jsonValue));
        const auto& obj = JsonParser::toObject(jsonValue);
        EXPECT_EQ(obj.members.size(), 2U);
        EXPECT_EQ(JsonParser::toInt(obj["a"]), 1);
        EXPECT_EQ(JsonParser::toInt(obj["b"]), 2);
    }
}

TEST(JsonParser, ParseNumberLimits) {
    {
        std::string_view jsonStr = "2147483647"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isInt(jsonValue));
        EXPECT_EQ(JsonParser::toInt(jsonValue), std::numeric_limits<int>::max());
    }

    {
        std::string_view jsonStr = "-2147483648"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isInt(jsonValue));
        EXPECT_EQ(JsonParser::toInt(jsonValue), std::numeric_limits<int>::min());
    }

    {
        std::string_view jsonStr = "1.7976931348623157E+308"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isDouble(jsonValue));
        EXPECT_NEAR(JsonParser::toDouble(jsonValue), std::numeric_limits<double>::max(), 0.001);
    }

    {
        std::string_view jsonStr = "-1.7976931348623157E+308"sv;
        JsonParser parser;
        JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonParser::isDouble(jsonValue));
        EXPECT_NEAR(JsonParser::toDouble(jsonValue), std::numeric_limits<double>::lowest(), 0.001);
    }
}

TEST(JsonParser, ParseComplexStructure) {
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
   )"sv;

    JsonParser parser;
    JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(JsonParser::isObject(jsonValue));
    const JsonParser::JsonObject& obj = JsonParser::toObject(jsonValue);

    EXPECT_EQ(obj["name"], "John Doe");
    EXPECT_TRUE(JsonParser::isInt(obj["age"]));
    EXPECT_EQ(JsonParser::toInt(obj["age"]), 30);
    EXPECT_TRUE(JsonParser::isDouble(obj["height"]));
    EXPECT_NEAR(JsonParser::toDouble(obj["height"]), 1.75, 0.001);
    EXPECT_TRUE(JsonParser::isBool(obj["married"]));
    EXPECT_EQ(JsonParser::toBool(obj["married"]), false);
    EXPECT_TRUE(JsonParser::isNull(obj["hobbies"]));

    EXPECT_TRUE(JsonParser::isObject(obj["address"]));
    const JsonParser::JsonObject& address = JsonParser::toObject(obj["address"]);
    EXPECT_EQ(address["street"], "123 Main St");
    EXPECT_EQ(address["city"], "New York");
    EXPECT_EQ(address["country"], "USA");
    EXPECT_EQ(address["zipcode"], "10001");

    EXPECT_TRUE(JsonParser::isArray(obj["phoneNumbers"]));
    const JsonParser::JsonArray& phoneNumbers = JsonParser::toArray(obj["phoneNumbers"]);
    EXPECT_EQ(phoneNumbers.elements.size(), 2U);
    EXPECT_TRUE(JsonParser::isObject(phoneNumbers.elements[0]));
    const JsonParser::JsonObject& phoneNumber1 = JsonParser::toObject(phoneNumbers.elements[0]);
    EXPECT_EQ(phoneNumber1["type"], "home");
    EXPECT_EQ(phoneNumber1["number"], "+1-555-123-4567");

    EXPECT_TRUE(JsonParser::isArray(obj["friends"]));
    const JsonParser::JsonArray& friends = JsonParser::toArray(obj["friends"]);
    EXPECT_EQ(friends.elements.size(), 2U);
    EXPECT_TRUE(JsonParser::isObject(friends.elements[0]));
    const JsonParser::JsonObject& friend1 = JsonParser::toObject(friends.elements[0]);
    EXPECT_EQ(friend1["name"], "Alice");
    EXPECT_TRUE(JsonParser::isInt(friend1["age"]));
    EXPECT_EQ(JsonParser::toInt(friend1["age"]), 28);
    EXPECT_TRUE(JsonParser::isArray(friend1["hobbies"]));
    const JsonParser::JsonArray& hobbies1 = JsonParser::toArray(friend1["hobbies"]);
    EXPECT_EQ(hobbies1.elements.size(), 2U);
    EXPECT_EQ(hobbies1.elements[0], "reading");
    EXPECT_EQ(hobbies1.elements[1], "painting");

    EXPECT_TRUE(JsonParser::isArray(obj["scores"]));
    const JsonParser::JsonArray& scores = JsonParser::toArray(obj["scores"]);
    EXPECT_EQ(scores.elements.size(), 5U);
    EXPECT_TRUE(JsonParser::isDouble(scores.elements[0]));
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[0]), 7.5, 0.001);
    EXPECT_TRUE(JsonParser::isDouble(scores.elements[1]));
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[1]), 8.2, 0.001);
    EXPECT_TRUE(JsonParser::isDouble(scores.elements[2]));
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[2]), 9.0, 0.001);
    EXPECT_TRUE(JsonParser::isDouble(scores.elements[3]));
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[3]), -3.14, 0.001);
    EXPECT_TRUE(JsonParser::isDouble(scores.elements[4]));
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[4]), 2.71828, 0.001);

    EXPECT_EQ(obj["description"], "Hello, world! 😊 これは日本語のテキストです。 🇯🇵");
    const auto escaped = obj["escape"];
    EXPECT_EQ(escaped, "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨");
    EXPECT_EQ(obj["url"], "https://example.com?q=test&limit=10");

    EXPECT_TRUE(JsonParser::isObject(obj["nested"]));
    const JsonParser::JsonObject& nested = JsonParser::toObject(obj["nested"]);
    EXPECT_TRUE(JsonParser::isArray(nested["arr"]));
    const JsonParser::JsonArray& nestedArr = JsonParser::toArray(nested["arr"]);
    EXPECT_EQ(nestedArr.elements.size(), 2U);
    EXPECT_TRUE(JsonParser::isInt(nestedArr.elements[0]));
    EXPECT_EQ(JsonParser::toInt(nestedArr.elements[0]), 1);
    EXPECT_TRUE(JsonParser::isArray(nestedArr.elements[1]));
    const JsonParser::JsonArray& nestedArr2 = JsonParser::toArray(nestedArr.elements[1]);
    EXPECT_EQ(nestedArr2.elements.size(), 2U);
    EXPECT_TRUE(JsonParser::isInt(nestedArr2.elements[0]));
    EXPECT_EQ(JsonParser::toInt(nestedArr2.elements[0]), 2);
    // ... (continue assertions for nested arrays)

    EXPECT_TRUE(JsonParser::isObject(nested["obj"]));
    const JsonParser::JsonObject& nestedObj = JsonParser::toObject(nested["obj"]);
    EXPECT_TRUE(JsonParser::isObject(nestedObj["a"]));
    const JsonParser::JsonObject& nestedObjA = JsonParser::toObject(nestedObj["a"]);
    EXPECT_TRUE(JsonParser::isObject(nestedObjA["b"]));
    const JsonParser::JsonObject& nestedObjB = JsonParser::toObject(nestedObjA["b"]);
    EXPECT_TRUE(JsonParser::isObject(nestedObjB["c"]));
    const JsonParser::JsonObject& nestedObjC = JsonParser::toObject(nestedObjB["c"]);
    EXPECT_TRUE(JsonParser::isObject(nestedObjC["d"]));
    const JsonParser::JsonObject& nestedObjD = JsonParser::toObject(nestedObjC["d"]);
    EXPECT_EQ(nestedObjD["e"], "nested");
}

TEST(JsonParser, ParseComplexJsonStructure) {
    std::string_view jsonStr = R"(
       {
           "name": "John Doe",
           "age": 30,
           "address": {
               "street": "123 Main St",
               "city": "New York",
               "country": "USA"
           },
           "phoneNumbers": [
               "555-1234",
               "555-5678"
           ],
           "email": null,
           "married": false,
           "children": [],
           "scores": [7.5, 8.2, 9.0],
           "description": "Hello, world! 😊 これは日本語のテキストです。 🇯🇵",
           "escaped": "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:\u2728"
       }
   )"sv;

    JsonParser parser;
    JsonParser::JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(JsonParser::isObject(jsonValue));
    const auto& obj = JsonParser::toObject(jsonValue);

    EXPECT_EQ(obj["name"], "John Doe");
    EXPECT_TRUE(JsonParser::toInt(obj["age"]) == 30);

    const auto& address = JsonParser::toObject(obj["address"]);
    EXPECT_EQ(address["street"], "123 Main St");
    EXPECT_EQ(address["city"], "New York");
    EXPECT_EQ(address["country"], "USA");

    const auto& phoneNumbers = JsonParser::toArray(obj["phoneNumbers"]);
    EXPECT_EQ(phoneNumbers.elements.size(), 2U);
    EXPECT_EQ(phoneNumbers.elements[0], "555-1234");
    EXPECT_EQ(phoneNumbers.elements[1], "555-5678");

    EXPECT_TRUE(JsonParser::isNull(obj["email"]));
    EXPECT_TRUE(JsonParser::toBool(obj["married"]) == false);

    const auto& children = JsonParser::toArray(obj["children"]);
    EXPECT_TRUE(children.elements.empty());

    const auto& scores = JsonParser::toArray(obj["scores"]);
    EXPECT_EQ(scores.elements.size(), 3U);
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[0]), 7.5, 0.001);
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[1]), 8.2, 0.001);
    EXPECT_NEAR(JsonParser::toDouble(scores.elements[2]), 9.0, 0.001);

    EXPECT_EQ(obj["description"], "Hello, world! 😊 これは日本語のテキストです。 🇯🇵");
    EXPECT_EQ(obj["escaped"], "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
