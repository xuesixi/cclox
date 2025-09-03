//
// Created by Yue Xue  on 9/3/25.
//

#include "cclox_util.h"
#include "typechecker/typechecker.h"
#include <catch2/catch_test_macros.hpp>

std::string read_file(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw FileOpenFailureError(fmt::format("the file: {} cannot be opened", path));
    }
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

TEST_CASE("basic primary") {
    TypeParser parser("int");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "int");
}

TEST_CASE("basic array") {
    TypeParser parser("int[]");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "int[]");
}

TEST_CASE("nested array") {
    TypeParser parser("int[][]");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "int[][]");
}

TEST_CASE("tuple of primitives") {
    TypeParser parser("(int, float)");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "(int, float)");
}

TEST_CASE("function type") {
    TypeParser parser("(int, float) -> int");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "(int, float) -> int");
}

TEST_CASE("function returning function") {
    TypeParser parser("(int) -> (float) -> int");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "(int) -> ((float) -> int)");
}

TEST_CASE("union type") {
    TypeParser parser("int | float");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "int | float");
}

TEST_CASE("intersection type") {
    TypeParser parser("Animal & Dog");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "Animal & Dog");
}

TEST_CASE("array of intersection") {
    TypeParser parser("(Animal & Dog)[]");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "(Animal & Dog)[]");
}

TEST_CASE("tuple with union element") {
    TypeParser parser("(int | float, string)");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "(int | float, string)");
}

TEST_CASE("union of arrays") {
    TypeParser parser("int[] | float[]");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "int[] | float[]");
}

TEST_CASE("complex function type") {
    TypeParser parser("((int | float)[], string) -> Animal & Dog");
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == "((int | float)[], string) -> Animal & Dog");
}


// int main() {
//     std::string src = read_file("/Users/yuexue/Codes/try/cclox/build/lox/type.lox");
//     TypeParser ast_builder(std::move(src));
//     auto type = ast_builder.parse_type();
//     std::cout << type->to_string();
// }