//
// Created by Yue Xue  on 9/6/25.
//

#include <catch2/catch_test_macros.hpp>
#include "typechecker/typeparser.h"

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
