//
// Created by Yue Xue  on 8/17/25.
//

#ifndef STRINGINTERN_H
#define STRINGINTERN_H

#include <vector>
#include <string>
#include <unordered_map>

namespace StringIntern {
    /**
     * 给定一个string名字，返回一个与它对应的id。
     * 如果该名字已存在，则直接返回其id。如果不存在，则将其添加入内部储存后，返回其id。
     */
    uint16_t resolve_string(const std::string &name);

    /**
     *
     * @param id 通过resolve_string获得的一个id
     * @return 该id对应的string名字
     */
    std::string read_from_id(uint16_t id);

    extern std::unordered_map<std::string, uint16_t> string_to_id;
    extern std::vector<std::string> id_to_string; // id就是索引
};

#endif //STRINGINTERN_H
