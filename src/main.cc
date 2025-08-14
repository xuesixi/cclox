//
// Created by Yue Xue  on 7/30/25.
//

#include "cclox_util.h"
#include "common.h"
#include "chunk.h"
#include "vm.h"
#include "scanner.h"
#include <error.h>
#include <memory>
#include "CLI11.hpp"

bool Flag::trace = false;
bool Flag::disassembly = false;
bool Flag::repl = false;

void repl() {
    std::string buffer;
    std::string last_time;
    VM vm;
    std::cout << "cclox repl start!\n";
    while (true) {
        if (!last_time.empty()) {
            std::cout << "... ";
        } else {
            std::cout << "> ";
        }
        std::getline(std::cin, buffer);
        if (buffer.empty()) {
            std::cout << "\n";
            return;
        }
        try {
            vm.interpret(last_time + buffer);
            last_time.clear();
        } catch ([[maybe_unused]] ConsumePending &pending) {
            last_time += buffer;
        } catch ([[maybe_unused]] CompilerError &error) {
            last_time.clear();
        }
    }
}

void run_file(const std::string &path) {
    VM vm;
    try {
        std::string source = read_file(path);
        vm.interpret(std::move(source));
    } catch (FileOpenFailureError &err) {
        std::cerr << err.what() << std::endl;
    }
}

void test_scanner(const std::string &path) {
    Scanner scanner(read_file(path));
    while (scanner.has_more()) {
        Token token = scanner.scan_token();
        std::cout << token.to_string() << std::endl;
    }
}

void go(int argc, const char **args) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(args[1]);
    } else {
        std::cerr << "error: expect zero or more arguments\n";
    }
}

int main(int argc, const char **args) {
    CLI::App app{"cclox description"};
    std::string filepath;

    app.add_option("-f, --file", filepath, "source file");
    app.add_flag("-t, --trace", Flag::trace, "trace each step");
    app.add_flag("-T, --disassembly", Flag::disassembly, "disassemble the byte codes");
    CLI11_PARSE(app, argc, args);
    if (filepath == "d") {
        // 默认的测试用文件, 以 cclox -f d 触发
        filepath = "/Users/yuexue/Codes/try/cclox/build/hello.lox";
    }
    if (filepath.empty()) {
        Flag::repl = true;
        repl();
    } else {
        run_file(filepath);
    }
}
