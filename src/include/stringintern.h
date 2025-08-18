//
// Created by Yue Xue  on 8/17/25.
//

#ifndef STRINGINTERN_H
#define STRINGINTERN_H

#include <vector>
#include <string>
#include <unordered_map>

namespace StringIntern {
    uint16_t resolve_string(const std::string &name);
    std::string read_from_id(uint16_t id);

    extern std::unordered_map<std::string, uint16_t> string_to_id;
    extern std::vector<std::string> id_to_string; // id就是索引
};

#endif //STRINGINTERN_H
