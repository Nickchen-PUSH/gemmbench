#include "cli.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "CLI11.hpp"
#include "../sample/sample_generator.h"
#include "../sample/sample_io.h"
#include "../sample/reference_gemm.h"
#include "../ops/registry.h"
#include "../benchmark/benchmark.h"
#include "../benchmark/verify.h"

int cli_main(int argc, char **argv)
{
    CLI::App app{"GEMM Benchmark Tool"};

    constexpr int kDefaultDim = 512;
    int M = kDefaultDim, N = kDefaultDim, K = kDefaultDim;
    int pattern = RANDOM;
    std::string pattern_str;
    std::string op_name;
    std::string output_json;
    std::string sample_out = "samples/default_sample.bin";
    std::string sample_in = sample_out;
    bool verbose = false;
    std::string verbose_matrix_file = "verbose_matrices.txt";

    // ---------- 子命令 generate ----------
    auto gen_cmd = app.add_subcommand("generate", "Generate test matrices");
    gen_cmd->add_option("--m", M, "M dimension")->default_val(std::to_string(M));
    gen_cmd->add_option("--n", N, "N dimension")->default_val(std::to_string(N));
    gen_cmd->add_option("--k", K, "K dimension")->default_val(std::to_string(K));
    gen_cmd->add_option("--type", pattern_str, "Type of matrix pattern: RANDOM, SEQUENTIAL, ONES, ZEROS, CUSTOM")
        ->default_val("RANDOM");
    gen_cmd->add_option("--sample", sample_out, "Path to save the generated sample")
        ->capture_default_str();

    // ---------- 子命令 run ----------
    auto run_cmd = app.add_subcommand("run", "Run GEMM benchmark");
    run_cmd->add_option("--op", op_name, "Operator name")->required();
    run_cmd->add_option("--output", output_json, "Output JSON file");
    run_cmd->add_option("--sample", sample_in, "Path to load the sample from")
        ->capture_default_str();

    run_cmd->add_option("--verbose", verbose, "Enable matrix printout for debugging");
    run_cmd->add_option("--verbose-matrix-file", verbose_matrix_file, "File to save verbose matrix output")
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
            std::cout << "Generating sample matrices with M=" << M << " N=" << N << " K=" << K
                      << ", pattern=" << pattern_str << "\n";
            if (pattern_str == "RANDOM")
            {
                pattern = RANDOM;
            }
            else if (pattern_str == "SEQUENTIAL")
            {
                pattern = SEQUENTIAL;
            }
            else if (pattern_str == "ONES")
            {
                pattern = ONES;
            }
            else if (pattern_str == "ZEROS")
            {
                pattern = ZEROS;
            }
            else if (pattern_str == "CUSTOM")
            {
                pattern = CUSTOM;
            }
            else
            {
                throw std::invalid_argument("Unknown pattern type: " + pattern_str);
            }
            SampleConfig cfg{M, N, K};
            auto A = generate_matrix(cfg.M, cfg.K, 42, pattern);
            auto B = generate_matrix(cfg.K, cfg.N, 1337, pattern);
            auto C = compute_reference_c(cfg, A, B);
            SampleData data{cfg, std::move(A), std::move(B), std::move(C)};
            save_sample_file(sample_out, data);
            std::cout << "Saved sample matrices to " << sample_out << "\n";
            std::cout << "A size: " << cfg.M << "x" << cfg.K
                      << ", B size: " << cfg.K << "x" << cfg.N
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

        if (op->columnMajor())
        {
            std::cout << "Converting sample matrices to column-major format for operator " << op_name << "\n";
            sample.convert_to_column_major();
        }

        const auto &cfg = sample.cfg;
        std::cout << "Running op=" << op_name
                  << " with M=" << cfg.M << " N=" << cfg.N << " K=" << cfg.K
                  << " from " << sample_in << "\n";

        auto computed = MatrixBuffer::allocate(static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.N));

        auto result = bench_gemm(op.get(), sample.A.data(), sample.B.data(), computed.data(),
                                 cfg.M, cfg.N, cfg.K);
        double flops = 2.0 * cfg.M * cfg.N * cfg.K;
        double gflops = flops / (result.ms * 1e-3 * 1e9);

        std::cout << "Time = " << result.ms << " ms\n";
        std::cout << "GFLOPS = " << gflops << "\n";

        auto verify = verify_result(sample.C.data(), computed.data(), cfg.M, cfg.N);
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
        if (verbose)
        {
            std::cout << "==============================\n";
            std::cout << "Matrix A:\n";
            sample.A.print(cfg.M, cfg.K, std::cout);
            std::cout << "------------------------------\n";
            std::cout << "Matrix B:\n";
            sample.B.print(cfg.K, cfg.N, std::cout);
            std::cout << "------------------------------\n";
            std::cout << "Reference Matrix C:\n";
            sample.C.print(cfg.M, cfg.N, std::cout);
            std::cout << "------------------------------\n";
            std::cout << "Computed Matrix C:\n";
            computed.print(cfg.M, cfg.N, std::cout);
            std::cout << "==============================\n";
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
            ofs << "  \"gflops\": " << gflops << ",\n";
            ofs << "  \"verified\": " << (verify.ok ? "true" : "false") << ",\n";
            ofs << "  \"max_abs_error\": " << verify.max_abs_error << ",\n";
            ofs << "  \"max_rel_error\": " << verify.max_rel_error << "\n";
            ofs << "}\n";
            ofs.close();
            std::cout << "Saved result to " << output_json << "\n";
            std::cout << "==============================\n";
        }

        return verify.ok ? 0 : 2;
    }

    return 0;
}