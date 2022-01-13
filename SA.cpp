#include "SA.h"
#include <regex>
#include <iostream>
#include "Token.h"

SA::SA(const std::vector<Token>& t) {
    tokens = t;
    FIRSTConstructor("FIRST.txt", first_);
	GetToken();
	try {
		Program();
	} catch (const Token& c) {
		std::cout << "ERROR: Undefined " << c.data << std::endl;
		return;
	}
	std::cout << "OK";
}

void SA::Definition() {
    Type();
    Name();

    if (cur.data != "=") throw cur;
    GetToken();

    Exp();

    while (cur.data == ",") {
        GetToken();
        Name();

        if (cur.data != "=") throw cur;
        GetToken();

        Exp();
    }
}

void SA::While() {
    if (cur.data != "while") throw cur;
    GetToken();

    if (cur.data != "(") throw cur;
    GetToken();

    Exp();

    if (cur.data != ")") throw cur;
    GetToken();

    Operator();
}

void SA::Enumeration() {
    Exp();

    while (cur.data == ",") {
        GetToken();
        Exp();
    }
}

void SA::Struct() {
    if (cur.data != "struct") throw cur;
    GetToken();

    Name();
    if (cur.data != "{") throw cur;
    GetToken();

    if (cur.data == "}") {
        GetToken();
        if (cur.data != ";") throw cur;
        GetToken();
        return;
    }

    Definition();
    while (cur.data == ",") {
        GetToken();
        Definition();
    }

    if (cur.data != "{") throw cur;
    GetToken();
    if (cur.data != ";") throw cur;
    GetToken();
    return;
}

void SA::Type() {
    if (cur.data == "int" || cur.data == "bool" ||
        cur.data == "char" || cur.data == "double") {
        GetToken();
        return;
    }
    throw cur;
}

void SA::Block() {
    if (cur.data != "{") throw cur;
    GetToken();

    while (first_equals("operator", cur.data))
        Operator();

    if (cur.data != "}") throw cur;
    GetToken();
    return;
}

void SA::For() {
    if (cur.data != "for") throw cur;
    GetToken();
    if (cur.data != "(") throw cur;
    GetToken();

    Definition();
    if (cur.data != ";") throw cur;
    GetToken();

    Exp();
    if (cur.data != ";") throw cur;
    GetToken();

    Exp();
    if (cur.data != ")") throw cur;
    GetToken();

    Operator();
}

void SA::If() {
    if (cur.data != "if") throw cur;
    GetToken();
    if (cur.data != "(") throw cur;
    GetToken();

    Exp();

    if (cur.data != ")") throw cur;
    GetToken();

    Operator();

    if (cur.data != "else") return;
    GetToken();

    Operator();
}

void SA::Operator() {
    if (first_equals("for", cur.data)) {
        For();
    } else if (first_equals("while", cur.data)) {
        While();
    } else if (first_equals("if", cur.data)) {
        If();
    } else if (first_equals("block", cur.data)) {
        Block();
    } else if (first_equals("exp", cur.data)) {
        Exp();
        if (cur.data != ";") throw cur;
        GetToken();
    } else {
        throw cur;
    }
}

void SA::Name() {
    if (cur.type == 1) { // 1 - Name
        GetToken();
        return;
    } else {
        throw cur;
    }
}

void SA::Params() {
    Type();
    Name();
    while (cur.data == ",") {
        GetToken();
        Params();
    }
    return;
}

void SA::Func() {
    Type();
    Name();

    if (cur.data != "(") throw cur;
    GetToken();

    if (cur.data != ")") {
        Params();
    }

    if (cur.data != ")") throw cur;
    GetToken();

    Block();
}

void SA::Import() {
    if (cur.data != "import") throw cur;
    GetToken();
    Name();
}

void SA::Program() {
    if (first_equals("type", cur.data)) {
        Func();
        Program();
        return;
    } else if (first_equals("struct", cur.data)) {
        Struct();
        Program();
        return;
    } else if (first_equals("import", cur.data)) {
        Import();
        Program();
        return;
    } else if (cur.data.empty()) {
        return;
    }
    throw cur;
}

void SA::GetToken() {
    if (ind == tokens.size()) {
        cur.data = "", cur.type = -1;
        return;
    }
    cur = tokens[ind++];
}

bool SA::first_equals(std::string str, std::string target) {
    if (str == "name") return cur.type == 1; // 1 - name
    if (str == "const") return cur.type == Integer || cur.type == Float;
    if (first_[str].empty()) return str == target;
    for (int i = 0; i < first_[str].size(); ++i) {
        if (first_equals(first_[str][i], target)) return true;
    }
    return false;
}

void SA::Prior1() {
    if (cur.data == "(") {
        GetToken();

        Exp();

        if (cur.data != ")") throw cur;
        GetToken();
    } else {
        Name();

        if (cur.data == "(") {
            GetToken();

            if (cur.data != ")") {
                Enumeration();
            }

            if (cur.data != ")") throw cur;
            GetToken();
        } else if (cur.data == "[") {
            GetToken();

            Exp();

            if(cur.data != "]") throw cur;
            GetToken();
        } else if (cur.data == ".") {
            GetToken();

            Prior1();
        }
    }
}

void SA::Prior2() {
    if (cur.data == "++" || cur.data == "--"
    || cur.data == "+" || cur.data == "-"
    || cur.data == "*" || cur.data == "&") {
        GetToken();
        Prior2();
    } else {
        if (first_equals("prior1", cur.data)) {
            Prior1();
        } else {
            if (cur.type != Integer && cur.type != Float) throw cur;
            GetToken();
        }
    }
}

void SA::Prior3() {
    Prior2();

    if (cur.data == "^") {
        GetToken();
        Prior3();
    }
}

void SA::Prior4() {
    Prior3();
    if (cur.data == "*" || cur.data == "/" || cur.data == "%") {
        GetToken();
        Prior4();
    }
}

void SA::Prior5() {
    Prior4();
    if (cur.data == "+" || cur.data == "-") {
        GetToken();
        Prior5();
    }
}

void SA::Prior6() {
    Prior5();
    if (cur.data == ">" || cur.data == "<" || cur.data == ">=" || cur.data == "<=") {
        GetToken();
        Prior6();
    }
}

void SA::Prior7() {
    Prior6();
    if (cur.data == "==" | cur.data == "!=") {
        GetToken();
        Prior7();
    }
}

void SA::Prior8() {
    Prior7();
    if (cur.data == "&") {
        GetToken();
        Prior8();
    }
}

void SA::Prior9() {
    Prior8();
    if (cur.data == "|") {
        GetToken();
        Prior9();
    }
}

void SA::Prior10() {
    Prior9();
    if (cur.data == "&&") {
        GetToken();
        Prior10();
    }
}

void SA::Prior11() {
    Prior10();
    if (cur.data == "||") {
        GetToken();
        Prior11();
    }
}

void SA::Prior12() {
    Prior11();
    if (cur.data == "=" || cur.data == "+=" || cur.data == "-="
    || cur.data == "*=" || cur.data == "/=" ||  cur.data == "%=" || cur.data == "^="
    || cur.data == "&=" || cur.data == "|=") {
        GetToken();
        Prior12();
    }
}

void SA::Exp() {
    Prior12();
    if (cur.data == ",") {
        GetToken();
        Exp();
    }
}


FIRSTConstructor::FIRSTConstructor(const std::string& filename, std::map<std::string, std::vector<std::string>> &f) {
    std::ifstream in(filename, std::ios::binary | std::ios::ate);
    auto size = in.tellg();
    std::string input(size, '\0');
    in.seekg(0);
    if (!in.read(&input[0], size)) {
        throw std::runtime_error("Can't open input file");
    }
    std::regex r(R"(\{([\w\?]+)\s?->\s?(.+)\})");
    auto beg = std::sregex_iterator(input.begin(), input.end(), r);
    auto end = std::sregex_iterator();
    for (auto it = beg; it != end; ++it) {
        std::smatch match = *it;
        std::string from = match[1].str();
        std::string to = match[2].str();
        std::string word = "";
        for (auto& x: to) {
            if (x == ' ') {
                if (!word.empty()) {
                    f[from].push_back(word);
                }
                word = "";
            } else {
                word += x;
            }
        }
        if (!word.empty()) {
            f[from].push_back(word);
        }
    }
}
