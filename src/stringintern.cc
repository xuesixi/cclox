//
// Created by Yue Xue  on 8/17/25.
//

#include "stringintern.h"

#include "cclox_util.h"
#include "error.h"

uint16_t StringIntern::resolve_string(const std::string &name) {
    auto found = string_to_id.find(name);
    if (found == string_to_id.end()) {
        id_to_string.push_back(name);
        size_t id = id_to_string.size() - 1;
        if (within<uint16_t>(id) == false) {
            throw StringInternOverflowError("String intern overflow");
        }
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
