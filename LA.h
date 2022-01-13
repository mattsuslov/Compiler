#ifndef LA_H_
#define LA_H_

#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <regex>
#include <fstream>
#include <map>
#include "Token.h"

struct Statement {
    Statement* next[256] = {};
    int term = NotTerm;
    int ret_type = Other;
};

class DFSMConstructor {
public:
    DFSMConstructor(const std::string& filename);

    Statement* getRoot() const {
        return root;
    }

private:
    Statement* root;

    int getTypeByName(const std::string& name) {
        if (name == "Res") return Res;
        if (name == "Var") return Name;
        if (name == "Float") return Float;
        if (name == "Integer") return Integer;
        if (name == "Oper") return Oper;
        if (name == "Punct") return Punct;
        if (name == "Other") return Other;
        return Other;
    }
};

class LA {
public:
	LA();

	std::vector<Token> fact_to_tokens() {
		std::vector<Token> res;
		int pos = 0;
		while (pos < input.size()) {
			auto tok = read_token(pos);
			if (tok.data == "" || tok.data[0] == 0 || tok.data == "\t") continue;
			res.push_back(tok); // @suppress("Invalid arguments")
		}
		return res;
	}

	void print_tokens(std::ostream& out, const std::vector<Token>& vec) const {
		for (Token tok: vec) {
			out << tok.data << " - " << get_type_name(tok.type) << '\n';
		}
	}

private:
	std::set<std::string> reserved_words;
	Statement* sfm = nullptr;
	std::string input = "";

	Token read_token(int& pos) {
		pos = std::max(0, pos);
		Token token;
		Statement* ptr = sfm;
		while (pos < input.size() && ptr && ptr->term == NotTerm) {
			token.data += input[pos];
			ptr = ptr->next[(u_char)input[pos]];
			++pos;
		}
		if (ptr && ptr->term == OneLetter) {
			--pos;
			token.data.pop_back();
		}
		if (ptr) token.type = ptr->ret_type;
		if (token.type == Name && reserved_words.count(token.data)) {
			token.type = Res;
		}
		return token;
	}

	std::string get_type_name(int type) const {
		switch (type) {
		case Res:
			return "Reserved word";
		case Name:
			return "Name";
		case Float:
			return "Float";
		case Integer:
			return "Integer";
		case Oper:
			return "Operator";
		case Punct:
			return "Punctuation";
		case Other:
			return "Other";
		default:
			break;
		}
		return "Unknown";
	}

	void read_code(std::istream& in) {
		unsigned long long size = in.tellg();
		input.assign(size + 1, '\0');
		in.seekg(0);
		if (!in.read(&input[0], size)) {
			throw std::runtime_error("Can't open input file");
		}
	}


	void clear_comments() {
		std::regex r = std::regex(R"((?://.*)|(/\*(?:.|[\n\r])*?\*/))");
		std::string out(input.size(), '\0');
		out[out.size() - 1] = '\n';
		std::regex_replace(&out[0], input.begin(), input.end(), r, "\n");
		input = out;
	}

	void load_input() {
		std::ifstream ifile("input.txt", std::ios::binary | std::ios::ate);
		read_code(ifile);
	}

	void load_reserved_words() {
		std::ifstream ifile("reserved.txt");
		std::string word;
		while (ifile >> word) {
			reserved_words.insert(word);
		}
	}
};

#endif /* LA_H_ */
