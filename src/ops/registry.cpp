#include "registry.h"
#include <iostream>

static std::unordered_map<std::string, GemmCreator> &op_registry()
{
    static std::unordered_map<std::string, GemmCreator> inst;
    return inst;
}

void register_op(const std::string &name, GemmCreator creator)
{
    op_registry()[name] = creator;
}

GemmOp *get_op(const std::string &name)
{
    auto &reg = op_registry();
    if (!reg.count(name))
        return nullptr;
    return reg[name]().release(); // 每次返回独立实例
}

std::vector<std::string> list_ops()
{
    std::vector<std::string> result;
    for (auto &p : op_registry())
        result.push_back(p.first);
    return result;
}