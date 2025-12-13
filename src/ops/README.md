# Operator Development Guide

## Adding a New GEMM Operator
1. **Create the implementation files**
   - Drop your sources into this folder (for example `fast_op.cpp`/`.h`).
   - Implement a class derived from `GemmOp` and override `run()`.
2. **Register the implementation**
   - Include `registry.h` in your `.cpp`.
   - Place `REGISTER_GEMM_OP(YourClassName)` at the bottom of the file so the static registrar runs.
3. **Hook into the build**
   - Open `CMakeLists.txt` in this directory and add a `register_op(...)` block that lists your source/headers and any extra compile flags or link libraries.
   - Re-run CMake (`cmake -B build && cmake --build build`). All registered operator object files are automatically added to the top-level `gemmbench` executable, so no manual linking is required.
4. **Validate**
   - Run `./bin/gemmbench list-ops` to confirm the operator shows up.
   - Use `./bin/gemmbench run --op YourClassName --sample <sample.bin>` to benchmark it.

## How Registration Works
- `registry.cpp` keeps a global `unordered_map<std::string, GemmCreator>` inside `op_registry()`.
- `REGISTER_GEMM_OP(Foo)` expands to a translation-unit static whose constructor calls `register_op("Foo", creator)`. This means simply linking the object file is enough to populate the registry before `main()` runs.
- Each call to `register_op` stores a lambda returning `std::unique_ptr<GemmOp>`. `get_op(name)` invokes the creator and returns a fresh instance.
- The `list_ops()` helper just iterates over the registry map and returns the collected keys; the CLI `list-ops` subcommand prints that list.
- Build glue: every `register_op()` call in `src/ops/CMakeLists.txt` creates an object library (`<name>_object`). The file also stores the target names in a global property. Later, `src/CMakeLists.txt` pulls those object files into the final `gemmbench` target so the static registrars from each operator are linked in automatically.
