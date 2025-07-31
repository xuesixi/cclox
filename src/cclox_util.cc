#include "cclox_util.h"
#include <stdexcept>
#include <fmt/core.h>

std::string read_file(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error(fmt::format("the file: {} cannot be opened", path));
    }
    return std::string(std::istreambuf_iterator<char>(ifs),std::istreambuf_iterator<char>());
}