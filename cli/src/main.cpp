#include "scanner.hpp"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <limits>

namespace {
void usage(std::ostream& out) {
    out << "DupliGuard - privacy-first duplicate file auditor\n\nUsage:\n  dupliguard scan DIRECTORY [--format text|json] [--output FILE]\n"
           "                  [--min-size BYTES] [--include-hidden]\n\nDupliGuard is read-only and never modifies scanned files.\n";
}
std::uintmax_t number(const std::string& text) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; })) {
        throw std::invalid_argument("invalid --min-size value");
    }
    size_t used=0; unsigned long long value;
    try { value=std::stoull(text,&used); } catch (...) { throw std::invalid_argument("invalid --min-size value"); }
    if(used!=text.size() || value>std::numeric_limits<std::uintmax_t>::max()) {
        throw std::invalid_argument("invalid --min-size value");
    }
    return value;
}
}
int main(int argc, char** argv) {
    try {
        if(argc==2 && std::string(argv[1])=="--help") { usage(std::cout); return 0; }
        if(argc<3 || std::string(argv[1])!="scan") { usage(std::cerr); return 2; }
        std::string format="text", output; std::uintmax_t min_size=1; bool include_hidden=false;
        for(int i=3;i<argc;++i){ std::string arg=argv[i];
            if(arg=="--include-hidden") include_hidden=true;
            else if((arg=="--format"||arg=="--output"||arg=="--min-size") && i+1<argc){ std::string value=argv[++i]; if(arg=="--format")format=value; else if(arg=="--output")output=value; else min_size=number(value); }
            else throw std::invalid_argument("unknown or incomplete option: "+arg);
        }
        if(format!="text"&&format!="json") throw std::invalid_argument("format must be text or json");
        auto report=dupliguard::scan(argv[2],min_size,include_hidden); auto rendered=format=="json"?dupliguard::to_json(report):dupliguard::to_text(report);
        if (output.empty()) {
            std::cout << rendered;
            if (!std::cout) throw std::runtime_error("cannot write report to stdout");
        } else {
            // Exclusive creation prevents report output from truncating scanned files or symlink targets.
            std::FILE* file = nullptr;
#ifdef _MSC_VER
            fopen_s(&file, output.c_str(), "wbx");
#else
            file = std::fopen(output.c_str(), "wbx");
#endif
            if (!file) throw std::runtime_error("cannot create output file (it must not already exist)");
            const bool write_failed = std::fwrite(rendered.data(), 1, rendered.size(), file) != rendered.size();
            const int close_result = std::fclose(file);
            if (write_failed || close_result != 0) throw std::runtime_error("cannot write output file");
        }
        return 0;
    } catch(const std::exception& e) { std::cerr<<"dupliguard: "<<e.what()<<'\n'; return 1; }
}
