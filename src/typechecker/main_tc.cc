//
// Created by Yue Xue  on 9/3/25.
//

#include "cclox_util.h"
#include "typechecker/typeparser.h"
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
    REQUIRE(type->to_string() == "(int) -> (float) -> int");
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
    std::string input = "((int | float)[], string) -> Animal & Dog";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("single primary type") {
    std::string input = "bool";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("single array of primary") {
    std::string input = "string[]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("double nested array") {
    std::string input = "float[][]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("triple nested array") {
    std::string input = "char[][][]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("tuple of three") {
    std::string input = "(int, float, string)";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("tuple with nested tuple") {
    std::string input = "((int, float), string)";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("union of three types") {
    std::string input = "int | float | string";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("intersection of three types") {
    std::string input = "A & B & C";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("union inside array") {
    std::string input = "(int | string)[]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("intersection inside array") {
    std::string input = "(X & Y)[]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function returning array") {
    std::string input = "(int) -> string[]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function taking array") {
    std::string input = "(int[]) -> bool";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function taking tuple") {
    std::string input = "((int, string)) -> float";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function returning function returning function") {
    std::string input = "(int) -> (float) -> (string) -> bool";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function with union return") {
    std::string input = "(int) -> int | string";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function with intersection parameter") {
    std::string input = "(A & B) -> C";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("union of arrays and primitives") {
    std::string input = "int | int[] | int[][]";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("tuple mixing union and intersection") {
    std::string input = "((A | B), (C & D))";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function with tuple of unions") {
    std::string input = "((int | float, string | char)) -> bool";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("function return tuple") {
    std::string input = "((int | float, string | char)) -> (bool, int)";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}

TEST_CASE("complex nested everything") {
    std::string input = "(((int | float)[], (Dog & Animal)[]) -> (Cat | Bird)[]) -> Zoo";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type = parser.parse_type();
    REQUIRE(type->to_string() == input);
}



// int main() {
//     std::string src = read_file("/Users/yuexue/Codes/try/cclox/build/lox/type.lox");
//     TypeParser ast_builder(std::move(src));
//     auto type = ast_builder.parse_type();
//     std::cout << type->to_string();
// }