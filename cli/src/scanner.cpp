#include "scanner.hpp"
#include "dupliguard/sha256.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;
namespace dupliguard {
namespace {
bool hidden(const fs::path& path) { auto name = path.filename().string(); return !name.empty() && name[0] == '.'; }

std::string file_hash(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot open file");
    dg_sha256_context ctx; dg_sha256_init(&ctx);
    char buffer[64 * 1024];
    while (input) {
        input.read(buffer, sizeof buffer);
        auto count = input.gcount();
        if (count > 0) dg_sha256_update(&ctx, reinterpret_cast<const uint8_t*>(buffer), static_cast<size_t>(count));
    }
    if (!input.eof()) throw std::runtime_error("cannot read file");
    uint8_t digest[DG_SHA256_DIGEST_SIZE]; char hex[DG_SHA256_HEX_SIZE];
    dg_sha256_final(&ctx, digest); dg_sha256_hex(digest, hex); return hex;
}

std::string json_escape(const std::string& value) {
    std::ostringstream out;
    for (unsigned char c : value) {
        switch (c) { case '\"': out << "\\\""; break; case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break; case '\f': out << "\\f"; break; case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break; case '\t': out << "\\t"; break;
        default: if (c < 0x20) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c; else out << c; }
    }
    return out.str();
}

std::string human(std::uintmax_t bytes) {
    static const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes); size_t unit = 0;
    while (value >= 1024.0 && unit < 4) { value /= 1024.0; ++unit; }
    std::ostringstream out; out << std::fixed << std::setprecision(unit ? 1 : 0) << value << ' ' << units[unit]; return out.str();
}
}

std::uint64_t Report::redundant_files() const { std::uint64_t n=0; for (const auto& g:groups) n += g.files.size()-1; return n; }
std::uintmax_t Report::reclaimable_bytes() const { std::uintmax_t n=0; for (const auto& g:groups) n += g.size*(g.files.size()-1); return n; }

Report scan(const fs::path& root, std::uintmax_t min_size, bool include_hidden) {
    auto started = std::chrono::steady_clock::now(); Report report; report.root = fs::absolute(root).lexically_normal();
    std::error_code ec;
    if (!fs::is_directory(report.root, ec) || ec) throw std::invalid_argument("scan path is not a readable directory");
    std::map<std::uintmax_t, std::vector<fs::path>> by_size;
    fs::recursive_directory_iterator it(report.root, fs::directory_options::skip_permission_denied, ec), end;
    if (ec) { ++report.errors; ec.clear(); }
    auto advance = [&] {
        it.increment(ec);
        if (ec) { ++report.errors; ec.clear(); }
    };
    while (it != end) {
        const auto entry = *it;
        if (!include_hidden && hidden(entry.path())) {
            if (entry.is_directory(ec)) it.disable_recursion_pending();
            if (ec) ++report.errors;
            ec.clear(); advance(); continue;
        }
        bool regular = entry.is_regular_file(ec); if (ec) { ++report.errors; ec.clear(); advance(); continue; }
        if (regular && !entry.is_symlink(ec)) {
            auto size = entry.file_size(ec);
            if (ec) { ++report.errors; ec.clear(); }
            else { ++report.files_scanned; report.bytes_scanned += size; if (size >= min_size) by_size[size].push_back(entry.path()); }
        }
        ec.clear(); advance();
    }
    for (auto& [size, paths] : by_size) if (paths.size() > 1) {
        std::map<std::string, std::vector<fs::path>> by_hash;
        for (const auto& path : paths) { try { by_hash[file_hash(path)].push_back(path); } catch (...) { ++report.errors; } }
        for (auto& [hash, files] : by_hash) if (files.size() > 1) {
            std::sort(files.begin(), files.end()); report.groups.push_back({hash, size, std::move(files)});
        }
    }
    std::sort(report.groups.begin(), report.groups.end(), [](const Group& a,const Group& b){ return a.size>b.size || (a.size==b.size && a.hash<b.hash); });
    report.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count();
    return report;
}

std::string to_json(const Report& r) {
    std::ostringstream o; o << "{\n  \"schemaVersion\": 1,\n  \"root\": \"" << json_escape(r.root.string()) << "\",\n"
      << "  \"summary\": {\"filesScanned\": " << r.files_scanned << ", \"bytesScanned\": " << r.bytes_scanned
      << ", \"duplicateGroups\": " << r.groups.size() << ", \"redundantFiles\": " << r.redundant_files()
      << ", \"reclaimableBytes\": " << r.reclaimable_bytes() << ", \"errors\": " << r.errors << ", \"elapsedMs\": " << r.elapsed_ms << "},\n  \"groups\": [";
    for (size_t i=0;i<r.groups.size();++i) {
        const auto& g=r.groups[i];
        o << (i?",\n":"\n") << "    {\"sha256\": \""<<g.hash<<"\", \"size\": "<<g.size<<", \"files\": [";
        for(size_t j=0;j<g.files.size();++j) {
            o << (j?", ":"") << "\"" << json_escape(g.files[j].string()) << "\"";
        }
        o << "]}";
    }
    if (!r.groups.empty()) {
        o << '\n';
    }
    o << "  ]\n}\n";
    return o.str();
}

std::string to_text(const Report& r) {
    std::ostringstream o; o << "Scanned "<<r.files_scanned<<" files ("<<human(r.bytes_scanned)<<") in "<<r.elapsed_ms<<" ms\nFound "<<r.groups.size()<<" duplicate groups, "<<r.redundant_files()<<" redundant copies, "<<human(r.reclaimable_bytes())<<" reclaimable\n";
    if(r.errors) o << "Warnings: "<<r.errors<<" filesystem entries could not be read\n";
    for(const auto& g:r.groups){ o << "\nSHA-256  "<<g.hash<<"  ("<<g.files.size()<<" files, "<<human(g.size)<<" each)\n"; for(const auto& f:g.files)o<<"  "<<f.string()<<'\n'; } return o.str();
}
}
