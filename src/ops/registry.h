#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "gemm_op.h"

using GemmCreator = std::function<std::unique_ptr<GemmOp>()>;

void register_op(const std::string &name, GemmCreator creator);

// 返回指向一个创建好的对象（每次调用返回独立实例）
GemmOp *get_op(const std::string &name);

// 列出所有已注册算子
std::vector<std::string> list_ops();

// 用于自动注册的宏
#define REGISTER_GEMM_OP(OP_CLASS)                                                     \
    namespace                                                                          \
    {                                                                                  \
        struct OP_CLASS##_registrar                                                    \
        {                                                                              \
            OP_CLASS##_registrar()                                                     \
            {                                                                          \
                register_op(#OP_CLASS, []() { return std::make_unique<OP_CLASS>(); }); \
            }                                                                          \
        };                                                                             \
        static OP_CLASS##_registrar global_##OP_CLASS##_registrar;                     \
    }