#include "json_writer.h"
#include <sstream>

std::string make_json(const BenchResult &r,
                      const std::string &op,
                      int M, int N, int K)
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"op\":\"" << op << "\",";
    oss << "\"M\":" << M << ",";
    oss << "\"N\":" << N << ",";
    oss << "\"K\":" << K << ",";
    oss << "\"time_ms\":" << r.ms;
    oss << "}";
    return oss.str();
}