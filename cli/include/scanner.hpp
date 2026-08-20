#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace dupliguard {
struct Group { std::string hash; std::uintmax_t size; std::vector<std::filesystem::path> files; };
struct Report {
    std::filesystem::path root;
    std::uint64_t files_scanned = 0, bytes_scanned = 0, errors = 0, elapsed_ms = 0;
    std::vector<Group> groups;
    std::uint64_t redundant_files() const;
    std::uintmax_t reclaimable_bytes() const;
};
Report scan(const std::filesystem::path& root, std::uintmax_t min_size, bool include_hidden);
std::string to_json(const Report& report);
std::string to_text(const Report& report);
}
