#include "SA.h"
#include <regex>
#include <iostream>
#include "Token.h"
#include "Error.h"
#include "Logger.h"

SA::SA() {
    FIRSTConstructor("FIRST.txt", first_);
}

void SA::Definition() {
    EEEType();
    Name();

    if (cur.data == "=") {
        GetToken();

        Prior12();
    }

    while (cur.data == ",") {
        GetToken();
        Name();

        if (cur.data == "=") {
            GetToken();

            Prior12();
        }
    }
}

void SA::While() {
    if (cur.data != "while") throw ExpectedSymbol(row, col, "while", cur.data.c_str());
    GetToken();

    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    Exp();

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
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
    if (cur.data != "struct") throw ExpectedSymbol(row, col, "struct", cur.data.c_str());
    GetToken();

    Name();
    if (cur.data != "{") throw ExpectedSymbol(row, col, "{", cur.data.c_str());
    GetToken();

    if (cur.data == "}") {
        GetToken();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
        return;
    }

    StructBody();

    if (cur.data != "}") throw ExpectedSymbol(row, col, "}", cur.data.c_str());
    GetToken();
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();
}

void SA::Type() {
    if (cur.data == "int" || cur.data == "bool" ||
        cur.data == "char" || cur.data == "double" || cur.data == "void" || cur.data == "float") {
        GetToken();
    } else if (first_equals("name", cur.data)) {
        Name();
    } else {
        throw Error(row, col, "Declared types have to begin with letter or _ -> " + cur.data);
    }
}

void SA::Block() {
    if (cur.data != "{") throw ExpectedSymbol(row, col, "{", cur.data.c_str());
    GetToken();

    while (first_equals("operator", cur.data))
        Operator();

    if (cur.data != "}") throw ExpectedSymbol(row, col, "}", cur.data.c_str());
    GetToken();
    return;
}

void SA::For() {
    if (cur.data != "for") throw ExpectedSymbol(row, col, "for", cur.data.c_str());
    GetToken();
    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    if (first_equals("exp", cur.data)) {
        Exp();
    } else {
        Definition();
    }
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();

    Exp();
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();

    Exp();
    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Operator();
}

void SA::If() {
    if (cur.data != "if") throw ExpectedSymbol(row, col, "if", cur.data.c_str());
    GetToken();
    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    Exp();

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Operator();

    if (cur.data != "else") return;
    GetToken();

    Operator();
}

void SA::Operator() {
    if (first_equals("name", cur.data)) {
        int mem_ind = ind;
        Token mem_tok = cur;
        Name();
        if (first_equals("name", cur.data) || cur.data == "&" || cur.data == "*" || cur.data == "[") {
            if (cur.data == "[") {
                GetToken();
                if (cur.data == "]") {
                    ind = mem_ind;
                    cur = mem_tok;
                    Definition();
                } else {
                    ind = mem_ind;
                    cur = mem_tok;
                    Exp();
                }
            } else {
                ind = mem_ind;
                cur = mem_tok;
                Definition();
            }
        } else {
            ind = mem_ind;
            cur = mem_tok;
            Enumeration();
        }
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
    } else if (first_equals("exp", cur.data)) {
        Enumeration();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
    } else if (first_equals("definition", cur.data)) {
        Definition();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
    } else if (first_equals("block", cur.data)) {
        Block();
    }  else if (first_equals("for", cur.data)) {
        For();
    } else if (first_equals("while", cur.data)) {
        While();
    } else if (first_equals("if", cur.data)) {
        If();
    } else if (first_equals("try", cur.data)) {
        Try();
    } else if (first_equals("_code_", cur.data)) {
        CodeBlock();
    } else if (first_equals("return", cur.data)) {
        Return();
    } else {
        throw Error(row, col, "Unknown operator -> " + cur.data);
    }
}

void SA::Name() {
    if (cur.type == 1) { // 1 - Name
        GetToken();
        return;
    } else {
        throw Error(row, col, "It is not name -> " + cur.data);
    }
}

void SA::Params() {
    EEEType();
    Name();
    while (cur.data == ",") {
        GetToken();
        Params();
    }
}

void SA::Func() {
    EEEType();
    Name();

    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    if (cur.data != ")") {
        Params();
    }

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Block();
}

void SA::Import() {
    if (cur.data != "import") throw ExpectedSymbol(row, col, "import", cur.data.c_str());
    GetToken();
    Name();
}

void SA::Program() {
    while (!cur.data.empty()) {
        if (first_equals("type", cur.data)) {
            int mem_ind = ind;
            Token mem_tok = cur;
            EEEType();
            Name();
            ind = mem_ind;
            if (cur.data == "(") {
                cur = mem_tok;
                Func();
            } else {
                cur = mem_tok;
                Definition();

                if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
                GetToken();
            }
        } else if (first_equals("struct", cur.data)) {
            Struct();
        } else if (first_equals("import", cur.data)) {
            Import();
        } else {
            throw ExpectedSymbol(row, col, "Incorrect program structure\n");
        }
    }

}

void SA::GetToken() {
    Token::mutex.lock();
    if (ind == Token::tokens.size() && Token::state == Go) {
        Token::state = Wait;
    }
    Token::mutex.unlock();
    while (Token::state == Wait) {}
    if (Token::state == End && ind == Token::tokens.size()) {
        cur.data = "", cur.type = -1;
        return;
    }
    Token::mutex.lock();
    cur = Token::tokens[ind++];
    Token::mutex.unlock();
    col += cur.data.size();
    if (cur.data == " ") {
        GetToken();
    }
    if (cur.data == "\n") {
        col = 1;
        row++;
        GetToken();
    }
    if (cur.type == Comment) {
        for (char ch: cur.data) {
            if (ch == '\n') {
                col = 1;
                row++;
            } else {
                col++;
            }
        }
        GetToken();
    }
}

bool SA::first_equals(const std::string& str, const std::string& target) {
    if (str == "name") return cur.type == 1;
    if (str == "const") return (cur.type == Integer || cur.type == Float || cur.type == String);
    if (str == target) return true;
    for (int i = 0; i < first_[str].size(); ++i) {
        auto& el = first_[str][i];
        if (el == "name" && cur.type == 1) return true;
        if (el == "const" && (cur.type == Integer || cur.type == Float || cur.type == String)) return true;
        if (el == target) return true;
    }
    return false;
}


void SA::Prior1() {
    if (cur.data == "(") {
        GetToken();

        Prior12();

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        GetToken();
    } else {
        Name();

        if (cur.data == "(") {
            GetToken();

            if (cur.data != ")") {
                Enumeration();
            }

            if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
            GetToken();
        } else if (cur.data == "[") {
            GetToken();

            Exp();

            if(cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
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
            if (cur.type != Integer && cur.type != Float && cur.type != String) throw ExpectedSymbol(row, col, "Constant", cur.data.c_str());
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
}

void SA::EType() {
    Type();
    while (cur.data == "[") {
        GetToken();
        if (cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
        GetToken();
    }
}

void SA::EEType() {
    EType();
    while (cur.data == "*") {
        GetToken();
    }
}

void SA::EEEType() {
    EEType();
    if (cur.data == "&&" || cur.data == "&") {
        GetToken();
    }
}

void SA::Return() {
    if (cur.data != "return") throw ExpectedSymbol(row, col, "return", cur.data.c_str());
    GetToken();

    if (cur.data != ";") Exp();
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();
}

void SA::StructBody() {
    int ind_mem = ind;
    Token tok_mem = cur;
    EEEType();
    Name();
    ind = ind_mem;
    if (cur.data == "(") {
        cur = tok_mem;
        Func();
    } else  {
        cur = tok_mem;
        Definition();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
    }
    if (cur.data == "}") return;
    StructBody();
}

void SA::Try() {
    if (cur.data != "try") throw ExpectedSymbol(row, col, "try", cur.data.c_str());
    GetToken();

    Block();
    if (cur.data != "catch") return;

    while (cur.data == "catch") {
        GetToken();
        if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
        GetToken();

        if (cur.data == ".") break;
        Definition();

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        GetToken();

        Block();
        if (cur.data != "catch") return;
    }

    for (int i = 0; i < 3; ++i) {
        if (cur.data != ".") throw ExpectedSymbol(row, col, ".", cur.data.c_str());
        GetToken();
    }

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Block();
}

void SA::CodeBlock() {
    if (cur.data != "_code_") throw ExpectedSymbol(row, col, "_code_", cur.data.c_str());
    GetToken();

    if (cur.data == "[") {
        GetToken();
        Exp();
        if (cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
        GetToken();

        if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
        GetToken();

        Name();

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        GetToken();
    } else {
        if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
        GetToken();

        Name();

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        GetToken();

        while (cur.data != "_endcode_") {
            GetToken();
            if (cur.type == -1) throw ExpectedSymbol(row, col, "_endcode_", cur.data.c_str());
        }
        GetToken();
    }
}

void SA::analize() {
    Logger::log("Anlizing...");
    auto st_time = std::chrono::high_resolution_clock::now();
    GetToken();
    try {
        Program();
    } catch (const Error& e) {
        std::cout << e;
        return;
    } catch (...) {
        std::cout << "ERROR: Undefined " << std::endl;
        return;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> tim = end_time - st_time;
    Logger::log("Anlizing time: " + std::to_string(tim.count()));
}


FIRSTConstructor::FIRSTConstructor(const std::string& filename, std::unordered_map<std::string, std::vector<std::string>> &f) {
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

    for (auto& p: f) {
        bool flag = true;
        while (flag) {
            flag = false;
            auto& vec = p.second;
            auto key = p.first;
            std::vector<std::string> nvec;
            for (const std::string& str: vec) {
                if (!f[str].empty()) {
                    flag = true;
                    for (int i = 0; i < f[str].size(); ++i) {
                        nvec.push_back(f[str][i]);
                    }
                } else {
                    nvec.push_back(str);
                }
            }
            f[key] = nvec;
        }
    }

}
