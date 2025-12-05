# GEMM Bench

GEMM Bench 是一个用于快速生成测试矩阵、运行多种 GEMM 算子并产出性能及正确性报告的 C++17 工具链。项目基于 CMake 构建，支持 FP32 / FP16 / BF16 等多种浮点精度，可用于算子验证、性能对比以及批量用例回归。

## 特性
- 🔁 **端到端工作流**：集成样本生成、参考结果、算子执行、误差校验与 JSON 报告。
- 🧮 **多精度支持**：统一的 `DataType` 抽象可以在不改算子实现的情况下切换 FP32 / FP16 / BF16。
- 🧱 **可扩展算子库**：通过 `REGISTER_GEMM_OP` 宏即可把自定义 GEMM 算子挂到 CLI 中进行测试。
- 📦 **可复用样本**：样本文件携带尺寸、dtype 以及参考 C 矩阵，可跨机器、跨运行复现测试。
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
| `generate` | 生成样本文件（包含 A/B/C） | `--m/--n/--k`，`--dtype float32|float16|bfloat16`，`--sample <path>` |
| `run` | 使用样本运行指定算子并输出性能/校验结果 | `--op <name>`，`--sample <path>`，`--output result.json` |
| `list-ops` | 列出已注册算子 | （无） |

查看已注册算子：
```bash
./bin/gemmbench list-ops
```

## 典型工作流
```bash
# 1. 生成一个 256³ 的 FP16 样本
./bin/gemmbench generate --m 256 --n 256 --k 256 --dtype fp16 --sample samples/fp16_256.bin

# 2. 运行算子并保存性能统计
./bin/gemmbench run --op NaiveGemmOp --sample samples/fp16_256.bin --output results/fp16_256_naive.json
```
CLI 会打印耗时（ms）、TFLOPS 以及最大绝对/相对误差。若 `--output` 提供了路径，将生成包含运行信息的 JSON。

## 多精度支持
- 所有样本与算子通过 `common/dtype.h` 中的 `DataType` 抽象访问矩阵数据。
- `SampleConfig::dtype` 指定当前样本的存储格式，默认 `float32`。
- `run` 命令在加载样本后会检查算子是否支持该 dtype，避免误用。
- 校验阶段默认阈值为 `atol = 1e-3`、`rtol = 1e-2`，可在需要时调整源码。

## 样本与结果
- 样本文件保存在 `samples/`（或 `cases/`）目录，内部包含魔数 `GSMM`、版本号（当前 `2`）、矩阵尺寸、dtype 以及顺序存储的 A/B/C。
- `cases/` 中给出了若干命名规范为 `case_${M}x${N}x${K}_${dtype}.bin` 的样本，可直接拿来跑基线。
- `scripts/case-run.sh` 会遍历尺寸×dtype×算子组合并把结果写入 `results/`。
- 结果 JSON 的字段包括：算子名、矩阵尺寸、`time_ms`、`tflops`、`verified` 以及误差统计。

## 目录结构
```
├── src
│   ├── cli/          # CLI11 命令行解析及子命令实现
│   ├── sample/       # 样本生成、序列化、参考 GEMM
│   ├── ops/          # GEMM 算子接口与注册表（默认包含 NaiveGemmOp）
│   ├── benchmark/    # bench harness 与结果校验
│   ├── output/       # JSON writer 等辅助模块
│   └── common/       # DataType 等共享工具
├── cases/            # 预制样本
├── results/          # 自动化脚本输出
├── scripts/          # case-run 等脚本
└── samples/          # 用户生成的样本默认目录
```

## 扩展算子
1. 在 `src/ops/` 下新增 `<your_op>.h/.cpp`，继承 `GemmOp` 并实现：
   - `std::string name() const`
   - `bool supports_dtype(DataType)`
   - `void run(const void* A, const void* B, void* C, int M, int N, int K, DataType dtype)`
2. 在实现文件尾部调用 `REGISTER_GEMM_OP(YourOp)` 进行自动注册。
3. 重新构建后即可通过 `--op YourOp` 在 CLI 中调用。

## 故障排查
- **Operator not found**：确认实现文件已被 CMake 捕获且含 `REGISTER_GEMM_OP`。
- **dtype mismatch**：`run` 时算子需返回 `supports_dtype(cfg.dtype)=true`；若不支持可回退到 FP32 样本。
- **Verification FAILED**：检查算子实现、确保样本与算子使用相同布局，并关注误差阈值是否需调大。
- **Sample file missing dtype information**：当前格式使用版本 2；若要加载旧样本，请确保 header version=1（默认视为 FP32）。

## 更多文档
详见 `docs/developer-guide.md`，包含架构、样本文件格式、JSON schema 以及扩展指南。
