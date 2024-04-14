#include <benchmark/benchmark.h>
#include <nlohmann/json.hpp>
#include <rapidjson/document.h>
#include "../auric_json.h"

const std::string kSmallJson = R"(
    {
        "name": "John Doe",
        "age": 30,
        "height": 1.75,
        "married": false,
        "hobbies": null
    }
)";

const std::string kMediumJson = R"(
    {
        "name": "Аліса Коваленко",
        "age": 28,
        "height": 1.68,
        "married": true,
        "hobbies": ["читання", "подорожі", "фотографія"],
        "address": {
            "street": "вул. Шевченка",
            "city": "Київ",
            "country": "Україна"
        },
        "phoneNumbers": [
            {
                "type": "домашній",
                "number": "+380441234567"
            },
            {
                "type": "мобільний",
                "number": "+380501234567"
            }
        ]
    }
)";

const std::string kLargeJson = R"(
    {
        "name": "张伟",
        "age": 35,
        "height": 1.8,
        "married": true,
        "spouse": {
            "name": "李娜",
            "age": 32,
            "height": 1.65,
            "hobbies": ["瑜伽", "园艺"]
        },
        "children": [
            {
                "name": "张明",
                "age": 5,
                "height": 1.1,
                "hobbies": ["画画", "足球"]
            },
            {
                "name": "张丽",
                "age": 3,
                "height": 0.95,
                "hobbies": ["唱歌", "跳舞"]
            }
        ],
        "parents": [
            {
                "name": "张刚",
                "age": 60,
                "height": 1.75,
                "hobbies": ["钓鱼", "收藏"]
            },
            {
                "name": "王芳",
                "age": 58,
                "height": 1.6,
                "hobbies": ["烹饪", "旅游"]
            }
        ],
        "workExperience": [
            {
                "company": "ABC科技公司",
                "position": "软件工程师",
                "startDate": "2010-07-01",
                "endDate": "2015-12-31",
                "responsibilities": [
                    "开发移动应用程序",
                    "优化应用程序性能",
                    "参与项目需求分析和设计"
                ]
            },
            {
                "company": "XYZ软件有限公司",
                "position": "高级软件工程师",
                "startDate": "2016-01-01",
                "endDate": null,
                "responsibilities": [
                    "领导团队开发大型项目",
                    "设计和实现系统架构",
                    "指导和培训初级工程师"
                ]
            }
        ],
        "education": {
            "degree": "学士",
            "major": "计算机科学与技术",
            "university": "北京大学",
            "graduationYear": 2010
        },
        "skills": ["Java", "Python", "C++", "数据结构与算法"],
        "languages": ["普通话", "英语", "日语"],
        "hobbies": ["摄影", "游泳", "电影"],
        "favoriteNumbers": [7, 3.14, 2.71828],
        "favoriteColors": ["绿色", "蓝色"],
        "favoriteFoods": ["火锅", "寿司"]
    }
)";

static void BM_AuricJson_ParseSmallJson(benchmark::State& state) {
    for (auto _ : state) {
        JsonParser parser;
        JsonParser::JsonValue json = parser.parse(kSmallJson);
        benchmark::DoNotOptimize(json);
    }
}

static void BM_NlohmannJson_ParseSmallJson(benchmark::State& state) {
    for (auto _ : state) {
        nlohmann::json json = nlohmann::json::parse(kSmallJson);
        benchmark::DoNotOptimize(json);
    }
}

static void BM_RapidJson_ParseSmallJson(benchmark::State& state) {
    for (auto _ : state) {
        rapidjson::Document json;
        json.Parse(kSmallJson.c_str());
        benchmark::DoNotOptimize(json);
    }
}

static void BM_AuricJson_ParseMediumJson(benchmark::State& state) {
    for (auto _ : state) {
        JsonParser parser;
        JsonParser::JsonValue json = parser.parse(kMediumJson);
        benchmark::DoNotOptimize(json);
    }
}

static void BM_NlohmannJson_ParseMediumJson(benchmark::State& state) {
    for (auto _ : state) {
        nlohmann::json json = nlohmann::json::parse(kMediumJson);
        benchmark::DoNotOptimize(json);
    }
}

static void BM_RapidJson_ParseMediumJson(benchmark::State& state) {
    for (auto _ : state) {
        rapidjson::Document json;
        json.Parse(kMediumJson.c_str());
        benchmark::DoNotOptimize(json);
    }
}

static void BM_AuricJson_ParseLargeJson(benchmark::State& state) {
    for (auto _ : state) {
        JsonParser parser;
        JsonParser::JsonValue json = parser.parse(kLargeJson);
        benchmark::DoNotOptimize(json);
    }
}

static void BM_NlohmannJson_ParseLargeJson(benchmark::State& state) {
    for (auto _ : state) {
        nlohmann::json json = nlohmann::json::parse(kLargeJson);
        benchmark::DoNotOptimize(json);
    }
}

static void BM_RapidJson_ParseLargeJson(benchmark::State& state) {
    for (auto _ : state) {
        rapidjson::Document json;
        json.Parse(kLargeJson.c_str());
        benchmark::DoNotOptimize(json);
    }
}

BENCHMARK(BM_AuricJson_ParseSmallJson);
BENCHMARK(BM_NlohmannJson_ParseSmallJson);
BENCHMARK(BM_RapidJson_ParseSmallJson);
BENCHMARK(BM_AuricJson_ParseMediumJson);
BENCHMARK(BM_NlohmannJson_ParseMediumJson);
BENCHMARK(BM_RapidJson_ParseMediumJson);
BENCHMARK(BM_AuricJson_ParseLargeJson);
BENCHMARK(BM_NlohmannJson_ParseLargeJson);
BENCHMARK(BM_RapidJson_ParseLargeJson);

BENCHMARK_MAIN();
