# Developer Guide

本文档深入说明 GEMM Bench 的架构、数据格式以及可扩展点，帮助你将其融入到更大的算子验证工作流中。

## 1. 模块概览

| 模块      | 位置              | 职责                                                                           |
| --------- | ----------------- | ------------------------------------------------------------------------------ |
| CLI       | `src/cli`       | 基于 CLI11 的命令行入口，暴露 `generate` / `run` / `list-ops` 子命令。   |
| Sample    | `src/sample`    | 负责样本配置、随机矩阵生成、参考 GEMM 以及样本序列化。                         |
| Ops       | `src/ops`       | 定义 `GemmOp` 接口并维护注册表，算子实现通过 `REGISTER_GEMM_OP` 自动挂载。 |
| Benchmark | `src/benchmark` | 执行算子、预热、计时以及 `verify_result` 精度校验。                          |
| Output    | `src/output`    | 生成 JSON 报告，方便与外部系统集成。                                           |
| Common    | `src/common`    | 提供 `DataType`、FP16/BF16 转换等共享基础设施。                              |
| Scripts   | `scripts/`      | 包含批量运行脚本，例如 `case-run.sh`。                                       |

数据流示意：

```
SampleConfig + DataType
      │
      ▼
 sample_generator -> sample_io (A/B/C + 元数据)
      │
      ▼
    CLI run -> load_sample -> bench_gemm -> verify_result -> JSON 输出
```

## 2. CLI 细节

### generate

- 参数：`--m`, `--n`, `--k`, `--dtype`, `--sample`
- 输出：包含三块数据的样本文件（详见第 3 节）。
- `SampleGenerator` 对 A/B 使用固定种子（123/456）和均匀分布 `[-1, 1]`，保证可重放。

### run

- 参数：`--op`, `--sample`, `--output`
- 步骤：
  1. 加载样本（自动判别 dtype）。
  2. 获取算子实例并检查 `supports_dtype`。
  3. 预热 3 次后计时一次，单位毫秒。
  4. 调用 `verify_result`（默认 `atol=1e-3`, `rtol=1e-2`）。
  5. （可选）写出 JSON 报告。

### list-ops

- 简单遍历注册表，可用于确认编译出的算子集合。

## 3. 样本文件格式

样本（`.bin`）在 `sample_io.cpp` 中定义，结构如下：

```
struct SampleFileHeaderBase {
    uint32_t magic;   // 固定 0x47534d4d ("GSMM")
    uint32_t version; // 当前为 2
    uint32_t M;
    uint32_t N;
    uint32_t K;
};
uint32_t dtype;       // DataType 枚举转成的整数
uint8_t  A[];         // M*K * dtype_size bytes
uint8_t  B[];         // K*N * dtype_size bytes
float    C[];         // M*N * sizeof(float)
```

- 版本 1 的旧文件不包含 `dtype` 字段，默认视为 `float32`。
- 保存前会校验 `A/B/C` 的字节数以避免损坏文件。

## 4. DataType 抽象

`common/dtype.h` 提供以下能力：

- `DataType` 枚举（FP32/FP16/BF16）及 `dtype_size` / `dtype_to_string` / `parse_dtype`。
- `load_value` / `store_value`（按 dtype 读写单个元素）。
- FP16 与 BF16 的打包/解包函数，遵循 IEEE754 取整规则。

所有算子实现都应通过这些 helper 访问输入/输出，以确保与样本格式对齐。

## 5. 添加新算子

1. 创建头文件并继承 `GemmOp`：
   ```cpp
   class FancyOp : public GemmOp {
   public:
       std::string name() const override { return "fancy"; }
       bool supports_dtype(DataType dt) const override { return dt == DataType::Float32; }
       void run(const void* A, const void* B, void* C,
                int M, int N, int K, DataType dtype) override;
   };
   ```
2. 在 `.cpp` 实现中使用 `load_value` / `store_value` 操作缓冲。
3. 在文件底部添加 `REGISTER_GEMM_OP(FancyOp);`。
4. 重新构建后通过 `./bin/gemmbench run --op FancyOp ...` 调用。

## 6. 批量运行与用例管理

- `cases/` 目录可存放预生成的样本，命名建议：`case_${M}x${N}x${K}_${dtype}.bin`。
- `scripts/case-run.sh` 会遍历 `sizes × dtypes × ops` 并执行多次 `run`，默认输出到 `results/`。
- 可根据需要修改脚本中的数组以覆盖新的尺寸、dtype 或算子。

## 7. JSON 输出格式

`run` 命令的 `--output` 生成如下键值：

```json
{
  "op": "NaiveGemmOp",
  "M": 256,
  "N": 256,
  "K": 256,
  "time_ms": 0.53,
  "tflops": 0.06,
  "verified": true,
  "max_abs_error": 2.3e-04,
  "max_rel_error": 1.2e-03
}
```

可直接解析并导入到可视化/数据库系统中；若需要额外字段（如硬件信息），可在 `cli.cpp` 的 `run` 分支中扩展输出逻辑。

## 8. 校验阈值

`verify_result` 的默认阈值为 `atol = 1e-3`, `rtol = 1e-2`，位于 `src/benchmark/verify.h`。如需放宽/收紧误差限制，可在调用点传入自定义参数，或在 CLI 中添加新选项。

## 9. 常见扩展方向

1. **更多 dtype**：在 `DataType` 枚举中新增条目，并补全转换函数与样本读写逻辑。
2. **更复杂的算子**：可在 `ops/` 中实现分块、SIMD、GPU 或外部库（如 cuBLAS）的包装。
3. **可视化/报告**：利用 JSON 输出接入 Prometheus、InfluxDB 或自定义仪表盘。
4. **CI 集成**：在流水线中运行 `scripts/case-run.sh` 并对比历史结果。

如需在文档中补充更多内容，可在 `docs/` 下继续添加 Markdown 文件，并在 README 的 “更多文档” 部分链接即可。
