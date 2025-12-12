# GEMM Bench

GEMM Bench 是一个用于快速生成测试矩阵、运行多种 GEMM 算子并产出性能及正确性报告的 C++17 工具链。项目基于 CMake 构建，专注于单精度（float32）场景，可用于算子验证、性能对比以及批量用例回归。

## 特性
- 🔁 **端到端工作流**：集成样本生成、参考结果、算子执行、误差校验与 JSON 报告。
- ⚙️ **轻量依赖**：所有数据以 float32 存储，省去了多精度转换、守卫逻辑。
- 🧱 **可扩展算子库**：通过 `REGISTER_GEMM_OP` 宏即可把自定义 GEMM 算子挂到 CLI 中进行测试。
- 📦 **可复用样本**：样本文件携带尺寸与参考 C 矩阵，可跨机器、跨运行复现测试。
- ⚙️ **自动化脚本**：`scripts/case-run.sh` 能够批量跑 `cases/` 下的样本并在 `results/` 中生成性能日志。

## 快速开始
### 依赖
- CMake ≥ 3.15
- Clang/GCC/MSVC (需支持 C++17)
- (可选) Ninja 以加速构建

### 构建
```bash
cmake -S . -B build
cmake --build build
```
二进制输出位于 `bin/gemmbench`。

### CLI 子命令
| 子命令 | 说明 | 常用选项 |
| --- | --- | --- |
| `generate` | 生成样本文件（包含 A/B/C） | `--m/--n/--k`，`--sample <path>` |
| `run` | 使用样本运行指定算子并输出性能/校验结果 | `--op <name>`，`--sample <path>`，`--output result.json` |
| `list-ops` | 列出已注册算子 | （无） |

查看已注册算子：
```bash
./bin/gemmbench list-ops
```

## 典型工作流
```bash
# 1. 生成一个 256³ 的样本
./bin/gemmbench generate --m 256 --n 256 --k 256 --sample samples/256.bin

# 2. 运行算子并保存性能统计
./bin/gemmbench run --op NaiveGemmOp --sample samples/256.bin --output results/256_naive.json
```
CLI 会打印耗时（ms）、TFLOPS 以及最大绝对/相对误差。若 `--output` 提供了路径，将生成包含运行信息的 JSON。

## 精度与误差
- 所有矩阵均以 float32 形式存储、传递与计算。
- 校验阶段默认阈值为 `atol = 1e-4`、`rtol = 1e-3`，如需调整可以修改 `verify_result` 的默认入参。

## 样本与结果
- 样本文件保存在 `samples/`（或 `cases/`）目录，内部包含魔数 `GSMM`、版本号（当前 `1`）、矩阵尺寸以及顺序存储的 float32 A/B/C。
- `cases/` 中给出了若干命名规范为 `case_${M}x${N}x${K}.bin`（或包含自定义后缀）的样本，可直接拿来跑基线。
- `scripts/case-run.sh` 会遍历尺寸×算子组合并把结果写入 `results/`。
- 结果 JSON 的字段包括：算子名、矩阵尺寸、`time_ms`、`tflops`、`verified` 以及误差统计。

## 目录结构
```
├── src
│   ├── cli/          # CLI11 命令行解析及子命令实现
│   ├── sample/       # 样本生成、序列化、参考 GEMM
│   ├── ops/          # GEMM 算子接口与注册表（默认包含 NaiveGemmOp）
│   ├── benchmark/    # bench harness 与结果校验
│   └── output/       # JSON writer 等辅助模块
├── cases/            # 预制样本
├── results/          # 自动化脚本输出
├── scripts/          # case-run 等脚本
└── samples/          # 用户生成的样本默认目录
```

## 扩展算子
1. 在 `src/ops/` 下新增 `<your_op>.h/.cpp`，继承 `GemmOp` 并实现：
   - `std::string name() const`
   - `void run(const float* A, const float* B, float* C, int M, int N, int K)`
2. 在实现文件尾部调用 `REGISTER_GEMM_OP(YourOp)` 进行自动注册。
3. 重新构建后即可通过 `--op YourOp` 在 CLI 中调用。

## 故障排查
- **Operator not found**：确认实现文件已被 CMake 捕获且含 `REGISTER_GEMM_OP`。
- **Unexpected NaNs/Inf**：检查算子实现是否对输入范围、初始化做了假设。
- **Verification FAILED**：检查算子实现、确保样本与算子使用相同布局，并关注误差阈值是否需调大。
- **Sample file corruption**：确保 header 中的 `M/N/K` 与文件大小一致，可重新生成样本修复。

## 更多文档
详见 `docs/developer-guide.md`，包含架构、样本文件格式、JSON schema 以及扩展指南。
