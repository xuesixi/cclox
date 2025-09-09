//
// Created by Yue Xue  on 7/30/25.
//

#include "cclox_util.h"
#include "common.h"
#include "chunk.h"
#include "vm.h"
#include "scanner.h"
#include "native.h"
#include <error.h>
#include <memory>
#include "CLI11.hpp"

namespace Flag {
    bool trace = false;
    bool disassembly = false;
    bool repl = false;
    bool show_heap = false;
    bool print_color = false;
    bool not_run = false;
    bool log_output = false;
}

namespace Configuration {
    int frame_max = 64;
}

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
        const auto result = vm.interpret(std::move(source));
        switch (result) {
            case InterpreterResult::CompileError: {
                print_to(std::cout, "== Compile Error ==\n", Color::RED);
                break;
            }
            case InterpreterResult::RuntimeError: {
                print_to(std::cout, "== Runtime Error ==\n", Color::RED);
                break;
            }
            case InterpreterResult::OK: {
                print_to(std::cout, "== Execution Finished ==\n", Color::GREEN);
                break;
            }
        }
    } catch (FileOpenFailureError &err) {
        std::cerr << err.what() << std::endl;
    }
}

void run_file_st(const std::string &path) {
    VM vm;
    try {
        std::string source = read_file(path);
        const auto result = vm.interpret_st(std::move(source));
        switch (result) {
            case InterpreterResult::CompileError: {
                print_to(std::cout, "== Compile Error ==\n", Color::RED);
                break;
            }
            case InterpreterResult::RuntimeError: {
                print_to(std::cout, "== Runtime Error ==\n", Color::RED);
                break;
            }
            case InterpreterResult::OK: {
                print_to(std::cout, "== Execution Finished ==\n", Color::GREEN);
                break;
            }
        }
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

int main(int argc, const char **args) {
    load_all_natives();
    CLI::App app{"cclox description"};
    std::string filepath;

    app.add_option("-f, --file", filepath, "source file");
    app.add_option("--frame-max", Configuration::frame_max, "the max amount of stack frames when running the vm");

    app.add_flag("-t, --trace", Flag::trace, "trace each step");
    app.add_flag("-l, --log", Flag::log_output, "each thread writes its output to a log file");
    app.add_flag("-H, --heap", Flag::show_heap, "show heap allocation info");
    app.add_flag("-T, --disassembly", Flag::disassembly, "disassemble the byte codes");
    app.add_flag("-n, --not-run", Flag::not_run, "don't run the code after the compilation");
    app.add_flag("-C, --color", Flag::print_color, "the result of print will be colored");

    CLI11_PARSE(app, argc, args);

    if (Flag::log_output) {
        Runtime::log_stream = std::ofstream("tmp_" + timestamp_str() + "_runtime.log");
        if (!Runtime::log_stream->is_open()) {
            throw FileOpenFailureError("cannot open the temporary runtime log file");
        }
    }
    if (filepath == "d") {
        // 默认的测试用文件, 以 cclox -f d 触发
        filepath = "/Users/yuexue/Codes/try/cclox/build/hello.lox";
    }
    if (filepath.empty()) {
        Flag::repl = true;
        repl();
    } else {
        run_file_st(filepath);
    }
}
