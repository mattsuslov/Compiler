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
    Statement* getRoot() const;

private:
    Statement* root;
    int getTypeByName(const std::string& name);
};

class LA {
public:
	LA();
    void fact_to_tokens();
	void print_tokens(std::ostream& out, const std::vector<Token>& vec) const;

    void operator()() {
        std::cout << "Factorization..." << std::endl;
        auto st_time = std::chrono::high_resolution_clock::now();

        fact_to_tokens();

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> tim = end_time - st_time;
        std::cout << std::fixed << "Factorization time: " << tim.count() << std::endl;
    }

private:
	std::set<std::string> reserved_words;
	Statement* sfm = nullptr;
	std::string input = "";
	Token read_token(int& pos);
	std::string get_type_name(int type) const;
	void read_code(std::istream& in);
	void clear_comments();
	void load_input();
	void load_reserved_words();
};

#endif /* LA_H_ */
