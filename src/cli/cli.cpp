#include "cli.h"

#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include <utility>
#include <cstdint>

#include "CLI11.hpp"
#include "../sample/sample_generator.h"
#include "../sample/sample_io.h"
#include "../sample/reference_gemm.h"
#include "../ops/registry.h"
#include "../benchmark/benchmark.h"
#include "../benchmark/verify.h"
#include "../common/dtype.h"

int cli_main(int argc, char **argv)
{
    CLI::App app{"GEMM Benchmark Tool"};

    constexpr int kDefaultDim = 512;
    int M = kDefaultDim, N = kDefaultDim, K = kDefaultDim;
    std::string dtype_name = "float32";
    std::string op_name;
    std::string output_json;
    std::string sample_out = "samples/default_sample.bin";
    std::string sample_in = sample_out;

    // ---------- 子命令 generate ----------
    auto gen_cmd = app.add_subcommand("generate", "Generate test matrices");
    gen_cmd->add_option("--m", M, "M dimension")->default_val(std::to_string(M));
    gen_cmd->add_option("--n", N, "N dimension")->default_val(std::to_string(N));
    gen_cmd->add_option("--k", K, "K dimension")->default_val(std::to_string(K));
    gen_cmd->add_option("--dtype", dtype_name, "Data type (float32, float16, bfloat16)");
    gen_cmd->add_option("--sample", sample_out, "Path to save the generated sample")
        ->capture_default_str();

    // ---------- 子命令 run ----------
    auto run_cmd = app.add_subcommand("run", "Run GEMM benchmark");
    run_cmd->add_option("--op", op_name, "Operator name")->required();
    run_cmd->add_option("--output", output_json, "Output JSON file");
    run_cmd->add_option("--sample", sample_in, "Path to load the sample from")
        ->capture_default_str();

    // ---------- 子命令 list-ops ----------
    auto list_cmd = app.add_subcommand("list-ops", "List available GEMM operators");

    app.require_subcommand(1); // 要求必须选一个子命令

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }

    // -------- generate 子命令逻辑 --------
    if (gen_cmd->parsed())
    {
        try
        {
            SampleConfig cfg{M, N, K};
            cfg.dtype = parse_dtype(dtype_name);
            SampleGenerator sg;
            auto A = sg.generateA(cfg);
            auto B = sg.generateB(cfg);
            auto C = compute_reference_c(cfg, A, B);
            SampleData data{cfg, std::move(A), std::move(B), std::move(C)};
            save_sample_file(sample_out, data);
            std::cout << "Saved sample matrices to " << sample_out << "\n";
            std::cout << "A size: " << cfg.M << "x" << cfg.K
                      << ", B size: " << cfg.K << "x" << cfg.N
                      << ", dtype=" << dtype_to_string(cfg.dtype)
                      << ", C reference computed" << "\n";
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to generate sample: " << ex.what() << "\n";
            return 1;
        }
        return 0;
    }

    if (list_cmd->parsed())
    {
        for (auto &name : list_ops())
        {
            std::cout << name << "\n";
        }
        return 0;
    }

    // -------- run 子命令逻辑 --------
    if (run_cmd->parsed())
    {
        std::unique_ptr<GemmOp> op(get_op(op_name));
        if (!op)
        {
            std::cerr << "Operator not found: " << op_name << "\n";
            return 1;
        }

        SampleData sample;
        try
        {
            sample = load_sample_file(sample_in);
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to load sample: " << ex.what() << "\n";
            return 1;
        }

        const auto &cfg = sample.cfg;
        if (!op->supports_dtype(cfg.dtype))
        {
            std::cerr << "Operator " << op_name << " does not support dtype "
                      << dtype_to_string(cfg.dtype) << "\n";
            return 1;
        }
        std::cout << "Running op=" << op_name
                  << " with M=" << cfg.M << " N=" << cfg.N << " K=" << cfg.K
                  << " dtype=" << dtype_to_string(cfg.dtype)
                  << " from " << sample_in << "\n";

        const std::size_t c_bytes = static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.N) * dtype_size(cfg.dtype);
        std::vector<std::uint8_t> computed(c_bytes);
        auto result = bench_gemm(op.get(), sample.A.data(), sample.B.data(), computed.data(),
                                 cfg.M, cfg.N, cfg.K, cfg.dtype);
        double flops = 2.0 * cfg.M * cfg.N * cfg.K;
        double tflops = flops / (result.ms * 1e6);

        std::cout << "Time = " << result.ms << " ms\n";
        std::cout << "TFLOPS = " << tflops << "\n";

        auto verify = verify_result(sample.C, computed.data(), cfg.M, cfg.N, cfg.dtype);
        if (verify.ok)
        {
            std::cout << "Verification PASSED. max_abs_err=" << verify.max_abs_error
                      << ", max_rel_err=" << verify.max_rel_error << "\n";
        }
        else
        {
            std::cerr << "Verification FAILED at (" << verify.mismatch_row
                      << ", " << verify.mismatch_col << ")"
                      << ". expected=" << verify.expected_value
                      << " actual=" << verify.actual_value
                      << " abs_err=" << verify.mismatch_abs_error
                      << " rel_err=" << verify.mismatch_rel_error << "\n";
        }

        if (!output_json.empty())
        {
            std::ofstream ofs(output_json);
            ofs << "{\n";
            ofs << "  \"op\": \"" << op_name << "\",\n";
            ofs << "  \"M\": " << cfg.M << ",\n";
            ofs << "  \"N\": " << cfg.N << ",\n";
            ofs << "  \"K\": " << cfg.K << ",\n";
            ofs << "  \"time_ms\": " << result.ms << ",\n";
            ofs << "  \"tflops\": " << tflops << ",\n";
            ofs << "  \"verified\": " << (verify.ok ? "true" : "false") << ",\n";
            ofs << "  \"max_abs_error\": " << verify.max_abs_error << ",\n";
            ofs << "  \"max_rel_error\": " << verify.max_rel_error << "\n";
            ofs << "}\n";
            ofs.close();
            std::cout << "Saved result to " << output_json << "\n";
        }

        return verify.ok ? 0 : 2;
    }

    return 0;
}