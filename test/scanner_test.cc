//
// Created by Yue Xue  on 8/15/25.
//

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <fmt/core.h>

#include "../src/include/scanner.h"
#include <sstream>

using Result = std::tuple<bool, size_t, size_t>;

std::string show(std::vector<Result> v) {
    std::stringstream s;
    for (auto [is_embeded, left, right]: v) {
        s << fmt::format("{}, [{}, {}]\n", is_embeded, left, right);
    }
    return s.str();
}

// Basic cases
TEST_CASE("empty string") {
    std::string s = "";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {};
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("no brackets") {
    std::string s = "hello world";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 10}  // "hello world" (0-based, end exclusive)
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("single bracket pair") {
    std::string s = "{content}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 8}  // "{content}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("empty brackets") {
    std::string s = "{}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 1}  // "{}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("multiple empty brackets") {
    std::string s = "{}{}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 1},   // first "{}"
        {true, 2, 3}    // second "{}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

// Mixed content cases
TEST_CASE("text before brackets") {
    std::string s = "prefix{content}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 5},   // "prefix"
        {true, 6, 14}    // "{content}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("text after brackets") {
    std::string s = "{content}suffix";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 8},    // "{content}"
        {false, 9, 14}   // "suffix"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("original example") {
    std::string s = "name{anda}end";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 3},   // "name"
        {true, 4, 9},    // "{anda}"
        {false, 10, 12}  // "end"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("multiple brackets with text") {
    std::string s = "a{b}c{d}e";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 0},   // "a"
        {true, 1, 3},    // "{b}"
        {false, 4, 4},   // "c"
        {true, 5, 7},    // "{d}"
        {false, 8, 8}    // "e"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("consecutive text sections") {
    std::string s = "hello{world}test{case}done";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 4},   // "hello"
        {true, 5, 11},   // "{world}"
        {false, 12, 15}, // "test"
        {true, 16, 21},  // "{case}"
        {false, 22, 25}  // "done"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

// Edge cases with whitespace
TEST_CASE("brackets with spaces") {
    std::string s = "{ spaces }";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 9}     // "{ spaces }"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("spaces between brackets") {
    std::string s = "{} {}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 1},    // "{}"
        {false, 2, 2},   // " "
        {true, 3, 4}     // "{}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

// Error cases - assuming these return std::nullopt
TEST_CASE("unmatched opening bracket") {
    std::string s = "test{unclosed";
    auto v = Scanner::split(s);
    REQUIRE_FALSE(v.has_value());
}

TEST_CASE("unmatched closing bracket") {
    std::string s = "test}unopened";
    auto v = Scanner::split(s);
    REQUIRE_FALSE(v.has_value());
}

TEST_CASE("multiple unmatched brackets") {
    std::string s = "{{{";
    auto v = Scanner::split(s);
    REQUIRE_FALSE(v.has_value());
}

TEST_CASE("closing before opening") {
    std::string s = "}test{";
    auto v = Scanner::split(s);
    REQUIRE_FALSE(v.has_value());
}

// Special characters
TEST_CASE("brackets with special characters") {
    std::string s = "{@#$%^&*()}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 10}    // "{@#$%^&*()}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("unicode content") {
    std::string s = "{üñíçødé}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, s.length() - 1}  // Entire string (careful with UTF-8 byte lengths)
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

// Performance/stress test
TEST_CASE("many alternating sections") {
    std::string s = "a{b}c{d}e{f}g{h}i{j}k";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 0},   // "a"
        {true, 1, 3},    // "{b}"
        {false, 4, 4},   // "c"
        {true, 5, 7},    // "{d}"
        {false, 8, 8},   // "e"
        {true, 9, 11},   // "{f}"
        {false, 12, 12}, // "g"
        {true, 13, 15},  // "{h}"
        {false, 16, 16}, // "i"
        {true, 17, 19},  // "{j}"
        {false, 20, 20}  // "k"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("only brackets") {
    std::string s = "{}{}{}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 1},    // first "{}"
        {true, 2, 3},    // second "{}"
        {true, 4, 5}     // third "{}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}
// Format string specific tests - these are more revealing for f-string usage
TEST_CASE("variable substitution pattern") {
    std::string s = "Hello {name}, you are {age} years old!";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 5},   // "Hello "
        {true, 6, 11},   // "{name}"
        {false, 12, 21}, // ", you are "
        {true, 22, 26},  // "{age}"
        {false, 27, 37}  // " years old!"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("expression-like variables") {
    std::string s = "Result: {obj.method()} or {arr[0]}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 7},   // "Result: "
        {true, 8, 21},   // "{obj.method()}"
        {false, 22, 25}, // " or "
        {true, 26, 33}   // "{arr[0]}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("format specifiers") {
    std::string s = "Pi: {pi:.2f}, Hex: {num:x}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 3},   // "Pi: "
        {true, 4, 11},   // "{pi:.2f}"
        {false, 12, 18}, // ", Hex: "
        {true, 19, 25}   // "{num:x}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("only variables") {
    std::string s = "{first}{second}{third}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 6},    // "{first}"
        {true, 7, 14},   // "{second}"
        {true, 15, 21}   // "{third}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("complex expressions") {
    std::string s = "Value is {calc(x + y * 2)} dollars";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 8},   // "Value is "
        {true, 9, 25},   // "{calc(x + y * 2)}"
        {false, 26, 33}  // " dollars"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("whitespace in expressions") {
    std::string s = "{ variable } and { other_var }";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {true, 0, 11},   // "{ variable }"
        {false, 12, 16}, // " and "
        {true, 17, 29}   // "{ other_var }"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("edge case: braces in strings") {
    // This might be invalid, but good to test behavior
    std::string s = "Text with } random brace and {var}";
    auto v = Scanner::split(s);
    // Behavior depends on implementation - might return nullopt or handle gracefully
    if (v) {
        // If it handles gracefully, check what it does
        REQUIRE(!v->empty());
    } else {
        // If it correctly identifies as invalid
        REQUIRE_FALSE(v.has_value());
    }
}

TEST_CASE("realistic log format") {
    std::string s = "[{timestamp}] {level}: {message} (user: {user_id})";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 0},   // "["
        {true, 1, 11},   // "{timestamp}"
        {false, 12, 13}, // "] "
        {true, 14, 20},  // "{level}"
        {false, 21, 22}, // ": "
        {true, 23, 31},  // "{message}"
        {false, 32, 39}, // " (user: "
        {true, 40, 48},  // "{user_id}"
        {false, 49, 49}  // ")"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("SQL-like template") {
    std::string s = "SELECT * FROM {table} WHERE id = {id} AND name = {name}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 13},  // "SELECT * FROM "
        {true, 14, 20},  // "{table}"
        {false, 21, 32}, // " WHERE id = "
        {true, 33, 36},  // "{id}"
        {false, 37, 48}, // " AND name = "
        {true, 49, 54}   // "{name}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("URL template") {
    std::string s = "https://api.{domain}/users/{user_id}/posts?page={page}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 11},  // "https://api."
        {true, 12, 19},  // "{domain}"
        {false, 20, 26}, // "/users/"
        {true, 27, 35},  // "{user_id}"
        {false, 36, 47}, // "/posts?page="
        {true, 48, 53}   // "{page}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("HTML template") {
    std::string s = "<h1>{title}</h1><p>Hello {name}!</p>";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 3},   // "<h1>"
        {true, 4, 10},   // "{title}"
        {false, 11, 24}, // "</h1><p>Hello "
        {true, 25, 30},  // "{name}"
        {false, 31, 35}  // "!</p>"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

// Performance/stress test for format strings
TEST_CASE("many variables template") {
    std::string s = "{a}{b}{c}{d}{e}{f}{g}{h}{i}{j}";
    auto v = Scanner::split(s);
    REQUIRE(v);
    REQUIRE(v->size() == 10);  // Should have 10 variable sections
    for (const auto& [is_embedded, left, right] : v.value()) {
        REQUIRE(is_embedded);  // All should be variables
    }
}

TEST_CASE("empty variable names") {
    std::string s = "Value: {}";  // Empty variable
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 6},   // "Value: "
        {true, 7, 8}     // "{}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("numeric-only variables") {
    std::string s = "Args: {0} and {1} and {10}";
    auto v = Scanner::split(s);
    std::vector<Result> expected = {
        {false, 0, 5},   // "Args: "
        {true, 6, 8},    // "{0}"
        {false, 9, 13},  // " and "
        {true, 14, 16},  // "{1}"
        {false, 17, 21}, // " and "
        {true, 22, 25}   // "{10}"
    };
    REQUIRE(v);
    REQUIRE(show(v.value()) == show(expected));
}

TEST_CASE("") {
    std::string s = "num is {add(1, 3)}, {10}";
}