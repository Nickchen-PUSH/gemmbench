#include "zsc_gemm.h"
#include "registry.h"

void ZscGemmOp::run(const float * /*A*/, const float * /*B*/, float * /*C*/,
                    int /*M*/, int /*N*/, int /*K*/)
{
    // TODO: implement
}

// 一行注册
REGISTER_GEMM_OP(ZscGemmOp)