// tests.cpp
#include <gtest/gtest.h>
#include <limits>
#include "../auric_json.h"

using namespace std::string_view_literals;

TEST(JsonParser, ParseNull) {
    std::string_view jsonStr = "null"sv;
    JsonParser parser;
    JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(jsonValue.value));
}

TEST(JsonParser, ParseBooleans) {
    {
        std::string_view jsonStr = "true"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<bool>(jsonValue.value));
        EXPECT_EQ(std::get<bool>(jsonValue.value), true);
    }

    {
        std::string_view jsonStr = "false"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<bool>(jsonValue.value));
        EXPECT_EQ(std::get<bool>(jsonValue.value), false);
    }
}

TEST(JsonParser, ParseNumbers) {
    {
        std::string_view jsonStr = "42"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<int>(jsonValue.value));
        EXPECT_EQ(std::get<int>(jsonValue.value), 42);
    }

    {
        std::string_view jsonStr = "-3.14"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<double>(jsonValue.value));
        EXPECT_NEAR(std::get<double>(jsonValue.value), -3.14, 0.001);
    }
}

TEST(JsonParser, ParseStrings) {
    {
        std::string_view jsonStr = R"("Hello, world!")"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<std::string>(jsonValue.value));
        EXPECT_EQ(std::get<std::string>(jsonValue.value), "Hello, world!");
    }
}

TEST(JsonParser, ParseUnicodeStrings) {
    {
        std::string_view jsonStr = R"("Hello, 世界!")"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isString(jsonValue.value));
        EXPECT_EQ(JsonValue::toString(jsonValue.value), "Hello, 世界!");
    }

    {
        std::string_view jsonStr = R"("Émoji 😃 Привіт")"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isString(jsonValue.value));
        EXPECT_EQ(JsonValue::toString(jsonValue.value), "Émoji 😃 Привіт");
    }

    {
        std::string_view jsonStr = R"("Escape sequences: \"\\/\b\f\n\r\t\u2028\u2029")"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isString(jsonValue.value));

        std::string expectedStr = "Escape sequences: \"\\/\b\f\n\r\t";
        expectedStr += static_cast<char>(0xE2);
        expectedStr += static_cast<char>(0x80);
        expectedStr += static_cast<char>(0xA8);
        expectedStr += static_cast<char>(0xE2);
        expectedStr += static_cast<char>(0x80);
        expectedStr += static_cast<char>(0xA9);

        EXPECT_EQ(JsonValue::toString(jsonValue.value), expectedStr);
    }

    {
        std::string_view jsonStr = R"("Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:\u2728")"sv;

        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isString(jsonValue.value));

        const auto str = JsonValue::toString(jsonValue.value);
        std::string expected = "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨";
        EXPECT_EQ(str, expected);
    }
}

TEST(JsonParser, ParseArrays) {
    {
        std::string_view jsonStr = R"([1, 2, 3])"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<JsonValue::Array>(jsonValue.value));
        const JsonValue::Array& arr = std::get<JsonValue::Array>(jsonValue.value);
        EXPECT_EQ(arr.elements.size(), 3U);
        EXPECT_TRUE(std::holds_alternative<int>(arr.elements[0].value));
        EXPECT_EQ(std::get<int>(arr.elements[0].value), 1);
        EXPECT_TRUE(std::holds_alternative<int>(arr.elements[1].value));
        EXPECT_EQ(std::get<int>(arr.elements[1].value), 2);
        EXPECT_TRUE(std::holds_alternative<int>(arr.elements[2].value));
        EXPECT_EQ(std::get<int>(arr.elements[2].value), 3);
    }
}

TEST(JsonParser, ParseObjects) {
    {
        std::string_view jsonStr = R"({"name": "John", "age": 30})"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(std::holds_alternative<JsonValue::Object>(jsonValue.value));
        const JsonValue::Object& obj = std::get<JsonValue::Object>(jsonValue.value);
        EXPECT_EQ(obj.members.size(), 2U);
        EXPECT_EQ(obj.members[0].first, "name");
        EXPECT_TRUE(std::holds_alternative<std::string>(obj.members[0].second.value));
        EXPECT_EQ(std::get<std::string>(obj.members[0].second.value), "John");
        EXPECT_EQ(obj.members[1].first, "age");
        EXPECT_TRUE(std::holds_alternative<int>(obj.members[1].second.value));
        EXPECT_EQ(std::get<int>(obj.members[1].second.value), 30);
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
    JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(std::holds_alternative<JsonValue::Object>(jsonValue.value));
    const JsonValue::Object& obj = std::get<JsonValue::Object>(jsonValue.value);
    EXPECT_EQ(obj.members.size(), 6U);

    EXPECT_EQ(obj.members[0].first, "name");
    EXPECT_TRUE(std::holds_alternative<std::string>(obj.members[0].second.value));
    EXPECT_EQ(std::get<std::string>(obj.members[0].second.value), "John");

    EXPECT_EQ(obj.members[1].first, "age");
    EXPECT_TRUE(std::holds_alternative<int>(obj.members[1].second.value));
    EXPECT_EQ(std::get<int>(obj.members[1].second.value), 30);

    EXPECT_EQ(obj.members[2].first, "address");
    EXPECT_TRUE(std::holds_alternative<JsonValue::Object>(obj.members[2].second.value));
    const JsonValue::Object& address = std::get<JsonValue::Object>(obj.members[2].second.value);
    EXPECT_EQ(address.members.size(), 2U);
    EXPECT_EQ(address.members[0].first, "street");
    EXPECT_TRUE(std::holds_alternative<std::string>(address.members[0].second.value));
    EXPECT_EQ(std::get<std::string>(address.members[0].second.value), "123 Main St");
    EXPECT_EQ(address.members[1].first, "city");
    EXPECT_TRUE(std::holds_alternative<std::string>(address.members[1].second.value));
    EXPECT_EQ(std::get<std::string>(address.members[1].second.value), "New York");

    EXPECT_EQ(obj.members[3].first, "hobbies");
    EXPECT_TRUE(std::holds_alternative<JsonValue::Array>(obj.members[3].second.value));
    const JsonValue::Array& hobbies = std::get<JsonValue::Array>(obj.members[3].second.value);
    EXPECT_EQ(hobbies.elements.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<std::string>(hobbies.elements[0].value));
    EXPECT_EQ(std::get<std::string>(hobbies.elements[0].value), "reading");
    EXPECT_TRUE(std::holds_alternative<std::string>(hobbies.elements[1].value));
    EXPECT_EQ(std::get<std::string>(hobbies.elements[1].value), "traveling");

    EXPECT_EQ(obj.members[4].first, "scores");
    EXPECT_TRUE(std::holds_alternative<JsonValue::Array>(obj.members[4].second.value));
    const JsonValue::Array& scores = std::get<JsonValue::Array>(obj.members[4].second.value);
    EXPECT_EQ(scores.elements.size(), 3U);
    EXPECT_TRUE(std::holds_alternative<double>(scores.elements[0].value));
    EXPECT_NEAR(std::get<double>(scores.elements[0].value), 7.5, 0.001);
    EXPECT_TRUE(std::holds_alternative<double>(scores.elements[1].value));
    EXPECT_NEAR(std::get<double>(scores.elements[1].value), 8.2, 0.001);
    EXPECT_TRUE(std::holds_alternative<double>(scores.elements[2].value));
    EXPECT_NEAR(std::get<double>(scores.elements[2].value), 9.0, 0.001);

    EXPECT_EQ(obj.members[5].first, "employed");
    EXPECT_TRUE(std::holds_alternative<bool>(obj.members[5].second.value));
    EXPECT_EQ(std::get<bool>(obj.members[5].second.value), true);
}

TEST(JsonParser, ParseEmptyArraysAndObjects) {
    {
        std::string_view jsonStr = "[]"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isArray(jsonValue.value));
        const auto& arr = JsonValue::toArray(jsonValue.value);
        EXPECT_TRUE(arr.elements.empty());
    }

    {
        std::string_view jsonStr = "{}"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isObject(jsonValue.value));
        const auto& obj = JsonValue::toObject(jsonValue.value);
        EXPECT_TRUE(obj.members.empty());
    }
}

TEST(JsonParser, ParseNumbersWithExponents) {
    {
        std::string_view jsonStr = "1.23e+4"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isDouble(jsonValue.value));
        EXPECT_NEAR(JsonValue::toDouble(jsonValue.value), 1.23e+4, 0.001);
    }

    {
        std::string_view jsonStr = "-5.67E-8"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isDouble(jsonValue.value));
        EXPECT_NEAR(JsonValue::toDouble(jsonValue.value), -5.67e-8, 0.001);
    }
}

TEST(JsonParser, ParseNumbersWithLeadingZeros) {
    {
        std::string_view jsonStr = "0123"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isInt(jsonValue.value));
        EXPECT_EQ(JsonValue::toInt(jsonValue.value), 123);
    }
}

TEST(JsonParser, ParseArrayWithTrailingComma) {
    {
        std::string_view jsonStr = "[1, 2, 3,]"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isArray(jsonValue.value));
        const auto& arr = JsonValue::toArray(jsonValue.value);
        EXPECT_EQ(arr.elements.size(), 3U);
        EXPECT_EQ(JsonValue::toInt(arr.elements[0]), 1);
        EXPECT_EQ(JsonValue::toInt(arr.elements[1]), 2);
        EXPECT_EQ(JsonValue::toInt(arr.elements[2]), 3);
    }
}

TEST(JsonParser, ParseObjectWithTrailingComma) {
    {
        std::string_view jsonStr = R"({"a": 1, "b": 2,})"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isObject(jsonValue.value));
        const auto& obj = JsonValue::toObject(jsonValue.value);
        EXPECT_EQ(obj.members.size(), 2U);
        EXPECT_EQ(JsonValue::toInt(obj["a"]), 1);
        EXPECT_EQ(JsonValue::toInt(obj["b"]), 2);
    }
}

TEST(JsonParser, ParseNumberLimits) {
    {
        std::string_view jsonStr = "2147483647"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isInt(jsonValue.value));
        EXPECT_EQ(JsonValue::toInt(jsonValue.value), std::numeric_limits<int>::max());
    }

    {
        std::string_view jsonStr = "-2147483648"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isInt(jsonValue.value));
        EXPECT_EQ(JsonValue::toInt(jsonValue.value), std::numeric_limits<int>::min());
    }

    {
        std::string_view jsonStr = "1.7976931348623157E+308"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isDouble(jsonValue.value));
        EXPECT_NEAR(JsonValue::toDouble(jsonValue.value), std::numeric_limits<double>::max(), 0.001);
    }

    {
        std::string_view jsonStr = "-1.7976931348623157E+308"sv;
        JsonParser parser;
        JsonValue jsonValue = parser.parse(jsonStr);
        EXPECT_TRUE(JsonValue::isDouble(jsonValue.value));
        EXPECT_NEAR(JsonValue::toDouble(jsonValue.value), std::numeric_limits<double>::lowest(), 0.001);
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
    JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(JsonValue::isObject(jsonValue.value));
    const JsonValue::Object& obj = JsonValue::toObject(jsonValue.value);

    EXPECT_EQ(obj["name"], "John Doe");
    EXPECT_TRUE(JsonValue::isInt(obj["age"]));
    EXPECT_EQ(JsonValue::toInt(obj["age"]), 30);
    EXPECT_TRUE(JsonValue::isDouble(obj["height"]));
    EXPECT_NEAR(JsonValue::toDouble(obj["height"]), 1.75, 0.001);
    EXPECT_TRUE(JsonValue::isBool(obj["married"]));
    EXPECT_EQ(JsonValue::toBool(obj["married"]), false);
    EXPECT_TRUE(JsonValue::isNull(obj["hobbies"]));

    EXPECT_TRUE(JsonValue::isObject(obj["address"]));
    const JsonValue::Object& address = JsonValue::toObject(obj["address"]);
    EXPECT_EQ(address["street"], "123 Main St");
    EXPECT_EQ(address["city"], "New York");
    EXPECT_EQ(address["country"], "USA");
    EXPECT_EQ(address["zipcode"], "10001");

    EXPECT_TRUE(JsonValue::isArray(obj["phoneNumbers"]));
    const JsonValue::Array& phoneNumbers = JsonValue::toArray(obj["phoneNumbers"]);
    EXPECT_EQ(phoneNumbers.elements.size(), 2U);
    EXPECT_TRUE(JsonValue::isObject(phoneNumbers.elements[0]));
    const JsonValue::Object& phoneNumber1 = JsonValue::toObject(phoneNumbers.elements[0]);
    EXPECT_EQ(phoneNumber1["type"], "home");
    EXPECT_EQ(phoneNumber1["number"], "+1-555-123-4567");

    EXPECT_TRUE(JsonValue::isArray(obj["friends"]));
    const JsonValue::Array& friends = JsonValue::toArray(obj["friends"]);
    EXPECT_EQ(friends.elements.size(), 2U);
    EXPECT_TRUE(JsonValue::isObject(friends.elements[0]));
    const JsonValue::Object& friend1 = JsonValue::toObject(friends.elements[0]);
    EXPECT_EQ(friend1["name"], "Alice");
    EXPECT_TRUE(JsonValue::isInt(friend1["age"]));
    EXPECT_EQ(JsonValue::toInt(friend1["age"]), 28);
    EXPECT_TRUE(JsonValue::isArray(friend1["hobbies"]));
    const JsonValue::Array& hobbies1 = JsonValue::toArray(friend1["hobbies"]);
    EXPECT_EQ(hobbies1.elements.size(), 2U);
    EXPECT_EQ(hobbies1.elements[0], "reading");
    EXPECT_EQ(hobbies1.elements[1], "painting");

    EXPECT_TRUE(JsonValue::isArray(obj["scores"]));
    const JsonValue::Array& scores = JsonValue::toArray(obj["scores"]);
    EXPECT_EQ(scores.elements.size(), 5U);
    EXPECT_TRUE(JsonValue::isDouble(scores.elements[0]));
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[0]), 7.5, 0.001);
    EXPECT_TRUE(JsonValue::isDouble(scores.elements[1]));
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[1]), 8.2, 0.001);
    EXPECT_TRUE(JsonValue::isDouble(scores.elements[2]));
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[2]), 9.0, 0.001);
    EXPECT_TRUE(JsonValue::isDouble(scores.elements[3]));
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[3]), -3.14, 0.001);
    EXPECT_TRUE(JsonValue::isDouble(scores.elements[4]));
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[4]), 2.71828, 0.001);

    EXPECT_EQ(obj["description"], "Hello, world! 😊 これは日本語のテキストです。 🇯🇵");
    const auto escaped = obj["escape"];
    EXPECT_EQ(escaped, "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨");
    EXPECT_EQ(obj["url"], "https://example.com?q=test&limit=10");

    EXPECT_TRUE(JsonValue::isObject(obj["nested"]));
    const JsonValue::Object& nested = JsonValue::toObject(obj["nested"]);
    EXPECT_TRUE(JsonValue::isArray(nested["arr"]));
    const JsonValue::Array& nestedArr = JsonValue::toArray(nested["arr"]);
    EXPECT_EQ(nestedArr.elements.size(), 2U);
    EXPECT_TRUE(JsonValue::isInt(nestedArr.elements[0]));
    EXPECT_EQ(JsonValue::toInt(nestedArr.elements[0]), 1);
    EXPECT_TRUE(JsonValue::isArray(nestedArr.elements[1]));
    const JsonValue::Array& nestedArr2 = JsonValue::toArray(nestedArr.elements[1]);
    EXPECT_EQ(nestedArr2.elements.size(), 2U);
    EXPECT_TRUE(JsonValue::isInt(nestedArr2.elements[0]));
    EXPECT_EQ(JsonValue::toInt(nestedArr2.elements[0]), 2);
    // ... (continue assertions for nested arrays)

    EXPECT_TRUE(JsonValue::isObject(nested["obj"]));
    const JsonValue::Object& nestedObj = JsonValue::toObject(nested["obj"]);
    EXPECT_TRUE(JsonValue::isObject(nestedObj["a"]));
    const JsonValue::Object& nestedObjA = JsonValue::toObject(nestedObj["a"]);
    EXPECT_TRUE(JsonValue::isObject(nestedObjA["b"]));
    const JsonValue::Object& nestedObjB = JsonValue::toObject(nestedObjA["b"]);
    EXPECT_TRUE(JsonValue::isObject(nestedObjB["c"]));
    const JsonValue::Object& nestedObjC = JsonValue::toObject(nestedObjB["c"]);
    EXPECT_TRUE(JsonValue::isObject(nestedObjC["d"]));
    const JsonValue::Object& nestedObjD = JsonValue::toObject(nestedObjC["d"]);
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
    JsonValue jsonValue = parser.parse(jsonStr);
    EXPECT_TRUE(JsonValue::isObject(jsonValue.value));
    const auto& obj = JsonValue::toObject(jsonValue.value);

    EXPECT_EQ(obj["name"], "John Doe");
    EXPECT_TRUE(JsonValue::toInt(obj["age"]) == 30);

    const auto& address = JsonValue::toObject(obj["address"]);
    EXPECT_EQ(address["street"], "123 Main St");
    EXPECT_EQ(address["city"], "New York");
    EXPECT_EQ(address["country"], "USA");

    const auto& phoneNumbers = JsonValue::toArray(obj["phoneNumbers"]);
    EXPECT_EQ(phoneNumbers.elements.size(), 2U);
    EXPECT_EQ(phoneNumbers.elements[0], "555-1234");
    EXPECT_EQ(phoneNumbers.elements[1], "555-5678");

    EXPECT_TRUE(JsonValue::isNull(obj["email"]));
    EXPECT_TRUE(JsonValue::toBool(obj["married"]) == false);

    const auto& children = JsonValue::toArray(obj["children"]);
    EXPECT_TRUE(children.elements.empty());

    const auto& scores = JsonValue::toArray(obj["scores"]);
    EXPECT_EQ(scores.elements.size(), 3U);
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[0]), 7.5, 0.001);
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[1]), 8.2, 0.001);
    EXPECT_NEAR(JsonValue::toDouble(scores.elements[2]), 9.0, 0.001);

    EXPECT_EQ(obj["description"], "Hello, world! 😊 これは日本語のテキストです。 🇯🇵");
    EXPECT_EQ(obj["escaped"], "Tab:\t Newline:\n Quote:\" Backslash:\\ Unicode:✨");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
