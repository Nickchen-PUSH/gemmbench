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
| Scripts   | `scripts/`      | 包含批量运行脚本，例如 `case-run.sh`。                                       |

数据流示意：

```
SampleConfig
      │
      ▼
 sample_generator -> sample_io (A/B/C + 元数据)
      │
      ▼
    CLI run -> load_sample -> bench_gemm -> verify_result -> JSON 输出
```

## 2. CLI 细节

### generate

- 参数：`--m`, `--n`, `--k`, `--sample`
- 输出：包含三块数据的样本文件（详见第 3 节）。
- `SampleGenerator` 对 A/B 使用固定种子（123/456）和均匀分布 `[-1, 1]`，保证可重放。

### run

- 参数：`--op`, `--sample`, `--output`
- 步骤：
  1. 加载样本（float32 格式）。
  2. 获取算子实例。
  3. 预热 3 次后计时一次，单位毫秒。
  4. 调用 `verify_result`（默认 `atol=1e-4`, `rtol=1e-3`）。
  5. （可选）写出 JSON 报告。

### list-ops

- 简单遍历注册表，可用于确认编译出的算子集合。

## 3. 样本文件格式

样本（`.bin`）在 `sample_io.cpp` 中定义，结构如下：

```
struct SampleFileHeaderBase {
    uint32_t magic;   // 固定 0x47534d4d ("GSMM")
    uint32_t version; // 当前为 1
    uint32_t M;
    uint32_t N;
    uint32_t K;
};
float    A[];         // M*K entries
float    B[];         // K*N entries
float    C[];         // M*N entries
```

- 全部矩阵都以 float32 顺序存储，`sample_io` 会校验尺寸与文件长度是否匹配。

## 4. 精度策略

- 工具链仅针对 float32，便于与参考实现保持一致，也避免了 dtype 兼容逻辑。
- 如果想支持其它精度，可以基于当前 float-only 代码继续扩展独立分支。

## 5. 添加新算子

1. 创建头文件并继承 `GemmOp`：
   ```cpp
   class FancyOp : public GemmOp {
   public:
       std::string name() const override { return "fancy"; }
  void run(const float* A, const float* B, float* C,
    int M, int N, int K) override;
   };
   ```
2. 在 `.cpp` 实现中直接按 float 指针访问数据；必要时可自定义块状读写函数。
3. 在文件底部添加 `REGISTER_GEMM_OP(FancyOp);`。
4. 重新构建后通过 `./bin/gemmbench run --op FancyOp ...` 调用。

## 6. 批量运行与用例管理

- `cases/` 目录可存放预生成的样本，命名建议：`case_${M}x${N}x${K}.bin` 或追加自定义后缀。
- `scripts/case-run.sh` 会遍历 `sizes × ops` 并执行多次 `run`，默认输出到 `results/`。
- 可根据需要修改脚本中的数组以覆盖新的尺寸或算子。

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

`verify_result` 的默认阈值为 `atol = 1e-4`, `rtol = 1e-3`，位于 `src/benchmark/verify.h`。如需放宽/收紧误差限制，可在调用点传入自定义参数，或在 CLI 中添加新选项。

## 9. 常见扩展方向

1. **更复杂的算子**：可在 `ops/` 中实现分块、SIMD、GPU 或外部库（如 cuBLAS）的包装。
2. **可视化/报告**：利用 JSON 输出接入 Prometheus、InfluxDB 或自定义仪表盘。
3. **CI 集成**：在流水线中运行 `scripts/case-run.sh` 并对比历史结果。

如需在文档中补充更多内容，可在 `docs/` 下继续添加 Markdown 文件，并在 README 的 “更多文档” 部分链接即可。
