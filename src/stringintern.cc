//
// Created by Yue Xue  on 8/17/25.
//

#include "include/stringintern.h"

uint16_t StringIntern::resolve_string(const std::string &name) {
    auto found = string_to_id.find(name);
    if (found == string_to_id.end()) {
        id_to_string.push_back(name);
        uint8_t id = id_to_string.size() - 1;
        string_to_id.insert({name, id});
        return id;
    } else {
        return found->second;
    }
}

std::string StringIntern::read_from_id(uint16_t id) {
    return id_to_string.at(id);
}

std::unordered_map<std::string, uint16_t> StringIntern::string_to_id;
std::vector<std::string> StringIntern::id_to_string; // id就是索引
