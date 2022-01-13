#include "LA.h"


LA::LA() {
	load_input();
	clear_comments();
	load_reserved_words();
	sfm = DFSMConstructor("DFSM.txt").getRoot();
}

DFSMConstructor::DFSMConstructor(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary | std::ios::ate);
    auto size = in.tellg();
    std::string input(size, '\0');
    in.seekg(0);
    if (!in.read(&input[0], size)) {
        throw std::runtime_error("Can't open input file");
    }
    std::map<std::string, Statement*> m;
    std::regex r(R"(\{([\w\?]+)\s?->\s?([\w\?]+)\s(.+)\})");
    auto beg = std::sregex_iterator(input.begin(), input.end(), r);
    auto end = std::sregex_iterator();
    for (auto it = beg; it != end; ++it) {
        std::smatch match = *it;
        std::string from = match[1].str();
        std::string to = match[2].str();
        std::string syms = match[3].str();
        if (!m[from]) m[from] = new Statement;
        if (!m[to]) m[to] = new Statement;
        Statement*& f = m[from];
        Statement*& t = m[to];
        if (syms == "ALL") {
            for (int i = 0; i < 256; ++i) {
                f->next[i] = t;
            }
        } else {
            for (u_char ch: syms) {
                f->next[ch] = t;
            }
        }
    }

    r = std::regex(R"(\{(\w+)\s-\s(\w+)\s-\s(\w+)\})");
    beg = std::sregex_iterator(input.begin(), input.end(), r);
    end = std::sregex_iterator();
    for (auto it = beg; it != end; ++it) {
        std::smatch match = *it;
        std::string state = match[1].str();
        std::string term = match[2].str();
        std::string ret_type = match[3].str();
        if (!m[state]) {
            throw std::runtime_error("Undefined state " + state);
        }
        Statement*& st = m[state];
        if (term == "OneLetter") {
            st->term = OneLetter;
            st->ret_type = getTypeByName(ret_type);
        } else if (term == "Nothing") {
            st->term = Nothing;
            st->ret_type = getTypeByName(ret_type);
        } else {
            continue;
        }
    }


    root = m["ROOT"];
}

