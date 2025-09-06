//
// Created by Yue Xue  on 9/3/25.
//

#include "cclox_util.h"
#include "typechecker/typeparser.h"
#include <catch2/catch_test_macros.hpp>

// 下面是测试类型解析器能否正确地解析类型

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

TEST_CASE("same type accept") {
    std::string input = "int int";
    auto copy = input;
    TypeParser parser(std::move(copy));
    auto type1 = parser.parse_type();
    auto type2 = parser.parse_type();
    REQUIRE(type1->accept(type2));
}

// 以下是关于类型的可接受性的判断测试
// 如果 type1.accept(type2) == true，那么 type1 是 type2 的父集、更广阔。

TEST_CASE("same primitive type") {
    TypeParser p1("int");
    TypeParser p2("int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("different primitive types reject") {
    TypeParser p1("int");
    TypeParser p2("float");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("union accepts member left") {
    TypeParser p1("int | float");
    TypeParser p2("int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("union accepts member right") {
    TypeParser p1("int | float");
    TypeParser p2("float");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("union rejects unrelated type") {
    TypeParser p1("int | float");
    TypeParser p2("string");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("array same element") {
    TypeParser p1("int[]");
    TypeParser p2("int[]");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("array different element reject") {
    TypeParser p1("int[]");
    TypeParser p2("float[]");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("tuple same arity and types") {
    TypeParser p1("(int, float)");
    TypeParser p2("(int, float)");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("tuple different arity reject") {
    TypeParser p1("(int, float)");
    TypeParser p2("(int, float, string)");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("function exact match") {
    TypeParser p1("(int, float) -> string");
    TypeParser p2("(int, float) -> string");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("function broader parameters") {
    TypeParser p1("(int) -> bool");
    TypeParser p2("(int | bool) -> bool");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("function narrower return type") {
    TypeParser p1("() -> (int | float)");
    TypeParser p2("() -> int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("function reject wrong parameter variance") {
    TypeParser p1("(int | float) -> bool");
    TypeParser p2("(int) -> bool");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2)); // narrower param can't accept broader
}

TEST_CASE("function reject wrong return variance") {
    TypeParser p1("() -> int");
    TypeParser p2("() -> (int | float)");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2)); // broader return not subtype of narrower
}

TEST_CASE("nested union inside array accept") {
    TypeParser p1("(int | float)[]");
    TypeParser p2("int[]");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("nested union inside array reject") {
    TypeParser p1("(int | float)[]");
    TypeParser p2("string[]");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("tuple with union element accept") {
    TypeParser p1("(int | float, string)");
    TypeParser p2("(int, string)");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("tuple with union element reject") {
    TypeParser p1("(int | float, string)");
    TypeParser p2("(string, string)");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("complex function reject") {
    TypeParser p1("((int | float), string) -> int");
    TypeParser p2("(int, string) -> (int | string)");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("complex function accept - fixed") {
    TypeParser p1("((int | float), string) -> (int | string)"); // contract
    TypeParser p2("((int | float), string) -> int");            // candidate
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2)); // params equal, return narrower -> ok
}

TEST_CASE("complex function reject - narrower parameter") {
    TypeParser p1("((int | float), string) -> (int | string)"); // contract
    TypeParser p2("(int, string) -> int");                      // candidate
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2)); // candidate param int can't accept float
}

TEST_CASE("parameters contravariant - accept") {
    TypeParser p1("(int) -> bool");            // contract expects int
    TypeParser p2("(int | float) -> bool");    // candidate accepts int OR float
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2)); // contract param <: candidate param -> ok
}

TEST_CASE("parameters contravariant - reject") {
    TypeParser p1("(int | float) -> bool"); // contract expects int|float
    TypeParser p2("(int) -> bool");         // candidate accepts only int
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2)); // candidate can't accept float -> reject
}

TEST_CASE("return covariance - accept") {
    TypeParser p1("() -> (int | float)"); // contract returns int|float
    TypeParser p2("() -> int");           // candidate returns int (narrower)
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2)); // candidate return <: contract return -> ok
}

TEST_CASE("return covariance - reject") {
    TypeParser p1("() -> int");          // contract returns int
    TypeParser p2("() -> (int | float)"); // candidate returns broader union
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2)); // broader return isn't subtype -> reject
}

TEST_CASE("any accepts primitive") {
    TypeParser p1("any");
    TypeParser p2("int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2)); // any accepts int
}

TEST_CASE("primitive does not accept any") {
    TypeParser p1("int");
    TypeParser p2("any");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2)); // int does not accept any
}

TEST_CASE("any accepts union") {
    TypeParser p1("any");
    TypeParser p2("int | float");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("union does not accept any") {
    TypeParser p1("int | float");
    TypeParser p2("any");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("any accepts array") {
    TypeParser p1("any");
    TypeParser p2("string[]");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("array does not accept any") {
    TypeParser p1("int[]");
    TypeParser p2("any");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("any accepts function") {
    TypeParser p1("any");
    TypeParser p2("(int) -> string");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("function does not accept any") {
    TypeParser p1("(int) -> string");
    TypeParser p2("any");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
}

TEST_CASE("any accepts any") {
    TypeParser p1("any");
    TypeParser p2("any");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("any in intersection") {
    TypeParser p1("int & any");
    TypeParser p2("int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
}

TEST_CASE("function param any - reject narrower param") {
    TypeParser p1("(any) -> bool");
    TypeParser p2("(int) -> bool");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
    REQUIRE(type2->accept(type1));
}

TEST_CASE("function return any - accept narrower return") {
    TypeParser p1("() -> any");
    TypeParser p2("() -> int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE(type1->accept(type2));
    REQUIRE_FALSE(type2->accept(type1));
}

TEST_CASE("function param and return any") {
    TypeParser p1("(any) -> any");
    TypeParser p2("(int) -> int");
    auto type1 = p1.parse_type();
    auto type2 = p2.parse_type();
    REQUIRE_FALSE(type1->accept(type2));
    REQUIRE_FALSE(type2->accept(type1));
}
