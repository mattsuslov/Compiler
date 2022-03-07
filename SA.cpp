#include "SA.h"
#include <regex>
#include <iostream>
#include "Token.h"
#include "Error.h"
#include "Logger.h"


SA::SA() {
    FIRSTConstructor("FIRST.txt", first_);
}

Semantic& sem = Semantic::inst();

void SA::Definition() {
    EEEType();
    Name();
    sem.push_type(sem.type);

    sem.put_id(sem.id, sem.type);

    if (cur.data == "=") {
        GetToken();

        Prior12();

        sem.check_op("=");
    }

    while (cur.data == ",") {
        GetToken();
        Name();

        sem.put_id(sem.id, sem.type);

        if (cur.data == "=") {
            GetToken();

            Prior12();

            sem.check_op("=");
        }
    }
}

void SA::While() {
    sem.extend_tid();
    if (cur.data != "while") throw ExpectedSymbol(row, col, "while", cur.data.c_str());
    GetToken();

    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    Exp();

    sem.eq_type(Semantic::Type(Semantic::BASE_TYPE::BOOL));

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    NonExtendedOperator();
    sem.del_tid();

}

std::vector<Semantic::Type> SA::Enumeration() {
    std::vector<Semantic::Type> res;

    Exp();
    res.push_back(sem.pop_type());

    while (cur.data == ",") {
        GetToken();
        Exp();
        res.push_back(sem.pop_type());
    }

    return res;
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
        if (cur.data == "int") sem.type = Semantic::Type(Semantic::BASE_TYPE::INT);
        if (cur.data == "bool") sem.type = Semantic::Type(Semantic::BASE_TYPE::BOOL);
        if (cur.data == "char") sem.type = Semantic::Type(Semantic::BASE_TYPE::CHAR);
        if (cur.data == "double") sem.type = Semantic::Type(Semantic::BASE_TYPE::DOUBLE);
        if (cur.data == "void") sem.type = Semantic::Type(Semantic::BASE_TYPE::VOID);
        if (cur.data == "float") sem.type = Semantic::Type(Semantic::BASE_TYPE::FLOAT);
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
}

void SA::For() {
    sem.extend_tid();
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
    sem.eq_type(Semantic::Type(Semantic::BASE_TYPE::BOOL));
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();

    Exp();
    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    NonExtendedOperator();
    sem.del_tid();
}

void SA::If() {
    sem.extend_tid();
    if (cur.data != "if") throw ExpectedSymbol(row, col, "if", cur.data.c_str());
    GetToken();
    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    Exp();
    sem.eq_type(Semantic::Type(Semantic::BASE_TYPE::BOOL));

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Operator();

    if (cur.data != "else") return;
    GetToken();

    Operator();
    sem.del_tid();
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
        sem.extend_tid();
        Block();
        sem.del_tid();
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
        sem.id = cur.data;
        GetToken();
        return;
    } else {
        throw Error(row, col, "It is not name -> " + cur.data);
    }
}


std::vector <Semantic::Type> SA::Params() {
    std::vector <Semantic::Type> res;
    EEEType();
    Name();

    sem.put_id(sem.id, sem.type);

    res.push_back(sem.type);
    while (cur.data == ",") {
        GetToken();
        auto tmp = Params();
        for (auto el : tmp) {
            res.push_back(el);
        }
    }
    return res;
}

void SA::Func() {
    EEEType();
    Name();

    std::string fname = sem.id;
    Semantic::Type ret_type = sem.type;
    sem.extend_tid();

    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();


    Semantic::FSignature tmp;
    tmp.ret_type = ret_type;
    if (cur.data != ")") {
        tmp.params = Params();
    }
    sem.put_ftid({fname, tmp});

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Block();

    sem.del_tid();
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

        std::string name = sem.id;

        if (cur.data == "(") {
            if (!sem.check_func(name)) throw Error("Undefined function name '" + name + "' ");
            GetToken();

            Semantic::FSignature res;
            if (cur.data != ")") {
                res.params = Enumeration();
            }

            if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
            GetToken();

            if (!sem.check_func(name, res)) throw Error("There isn't any function called '" + name + "' with such parameters");
            sem.push_type(sem.getSignature(name, res).ret_type);

        } else if (cur.data == "[") {
            while (cur.data == "[") {
                Semantic::Type type = sem.check_id(name);
                if (type.type.empty()) throw Error("Undefined var '" + sem.id + "' ");

                GetToken();

                Exp();

                if(cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
                GetToken();
            }
        } else if (cur.data == ".") {
            Semantic::Type type = sem.check_id(name);
            if (type.type.empty()) throw Error("Undefined var '" + sem.id + "' ");

            GetToken();

            Prior1();
        } else {
            Semantic::Type type = sem.check_id(name);
            if (type.type.empty()) throw Error("Undefined var '" + sem.id + "' ");

            sem.push_type(type);
        }
    }
}

void SA::Prior2() {
    if (cur.data == "++" || cur.data == "--"
    || cur.data == "+" || cur.data == "-"
    || cur.data == "*" || cur.data == "&") {
        std::string op = cur.data;
        op += "un";
        GetToken();
        Prior2();

        sem.check_op(op);
    } else {
        if (first_equals("prior1", cur.data)) {
            Prior1();
        } else {
            if (cur.type == Integer) {
                sem.push_type(Semantic::Type(Semantic::BASE_TYPE::INT));
            } else if (cur.type == Float) {
                sem.push_type(Semantic::Type(Semantic::BASE_TYPE::FLOAT));
            } else if (cur.type == String) {
                sem.push_type(Semantic::Type(Semantic::BASE_TYPE::STRING));
            } else if (cur.type == Res && (cur.data == "true" || cur.data == "false") ) {
                sem.push_type(Semantic::Type(Semantic::BASE_TYPE::BOOL));
            } else {
                throw ExpectedSymbol(row, col, "Constant", cur.data.c_str());
            }
            GetToken();
        }
    }
}


void SA::Prior3() {
    Prior2();

    if (cur.data == "^") {
        std::string op = cur.data;
        GetToken();
        Prior3();
        sem.check_op(op);
    }
}

void SA::Prior4() {
    Prior3();
    if (cur.data == "*" || cur.data == "/" || cur.data == "%") {
        std::string op = cur.data;
        GetToken();
        Prior4();

        sem.check_op(op);
    }
}

void SA::Prior5() {
    Prior4();
    if (cur.data == "+" || cur.data == "-") {
        std::string op = cur.data;
        GetToken();
        Prior5();

        sem.check_op(op);
    }
}

void SA::Prior6() {
    Prior5();
    if (cur.data == ">" || cur.data == "<" || cur.data == ">=" || cur.data == "<=") {
        std::string op = cur.data;
        GetToken();
        Prior6();
        sem.check_op(op);
    }
}

void SA::Prior7() {
    Prior6();
    if (cur.data == "==" | cur.data == "!=") {
        std::string op = cur.data;
        GetToken();
        Prior7();
        sem.check_op(op);
    }
}

void SA::Prior8() {
    Prior7();
    if (cur.data == "&") {
        GetToken();
        Prior8();
        sem.check_op("&");
    }
}

void SA::Prior9() {
    Prior8();
    if (cur.data == "|") {
        GetToken();
        Prior9();
        sem.check_op("|");
    }
}

void SA::Prior10() {
    Prior9();
    if (cur.data == "&&") {
        GetToken();
        Prior10();
        sem.check_op("&&");
    }
}

void SA::Prior11() {
    Prior10();
    if (cur.data == "||") {
        GetToken();
        Prior11();
        sem.check_op("||");
    }
}

void SA::Prior12() {
    Prior11();
    if (cur.data == "=" || cur.data == "+=" || cur.data == "-="
    || cur.data == "*=" || cur.data == "/=" ||  cur.data == "%=" || cur.data == "^="
    || cur.data == "&=" || cur.data == "|=") {
        std::string op = cur.data;
        GetToken();
        Prior12();
        sem.check_op(op);
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
        sem.type = Semantic::Type(Semantic::BASE_TYPE::POINTER);
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
        sem.extend_tid();

        GetToken();
        if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
        GetToken();

        if (cur.data == ".") break;
        Definition();

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        GetToken();

        Block();
        if (cur.data != "catch") return;

        sem.del_tid();
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
//    Program();
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

void SA::NonExtendedOperator() {
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

void Semantic::check_op(const std::string &op) {
    if (op == "+" || op == "*" || op == "/" || op == "-" || op == "^") {
        if (st.size() < 2) throw Error(op + " is a binary operation!!!");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1.type.top() == INT && t2.type.top() == INT) {
            st.push(Type(INT));
        } else {
            throw Error("Operands must be integers");
        }
    } else if (op == "+un" || op == "-un" || op == "++un" || op == "--un" || op == "*" || op == "&") {
        if (st.size() < 1) throw Error(op + " expected at least one operand!!!");
        Type t1 = st.top(); st.pop();
        if (t1.type.top() == INT) {
            st.push(Type(INT));
        } else {
            throw Error("Operands must be integers");
        }
    } else if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        if (st.size() < 2) throw Error(op + " is a binary operation!!!");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1.type.top() == BOOL && t2.type.top() == BOOL) {
            st.push(Type(BOOL));
        } else if (t1.type.top() == INT && t2.type.top() == INT) {
            st.push(Type(BOOL));
        } else {
            throw Error("Operands must be bool");
        }
    } else if (op == "=" || op == "+=" || op == "-="
               || op == "*=" || op == "/=" ||  op == "%=" || op == "^="
               || op == "&=" || op == "|=") {
        if (st.size() < 2) throw Error(op + " is a binary operation!!!");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1.type.top() == t2.type.top()) {
            st.push(t1.type.top());
        } else {
            throw Error("You can't put " + std::to_string(t1.type.top()) + " into " + std::to_string(t2.type.top()));
        }
    } else if (op == "|" || op == "&") {
        if (st.size() < 2) throw Error(op + " is a binary operation!!!");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1.type.top() == INT && t2.type.top() == INT) {
            st.push(Type(INT));
        } else {
            throw Error("Operands must be int");
        }
    } else if (op == "||" || op == "&&") {
        if (st.size() < 2) throw Error(op + " is a binary operation!!!");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1.type.top() == BOOL && t2.type.top() == BOOL) {
            st.push(Type(BOOL));
        } else {
            throw Error("Operands must be bool");
        }
    } else {
        throw Error("Unknown operation");
    }
}

void Semantic::put_id(const std::string &id, Semantic::Type type) {
    if (cur_tid->data.count(id)) throw Error(id + " has been already defined");
    cur_tid->data[id] = type;
}



void Semantic::eq_type(Semantic::Type type) {
    if (st.empty()) return;

 //   std::cout << type->type << " " << st.top()->type << '\n';
    if (type.type != st.top().type) {
        throw Error("Expected bool, but XXXX found");
    }
}

void Semantic::put_ftid(const std::pair<std::string, FSignature>& id) {
    if (ftid.count(id.first) && isContainSignature(ftid[id.first], id.second))
        throw Error("Ambiguous definition " + id.first);

    ftid[id.first].push_back(id.second);
}

Semantic::Type Semantic::pop_type() {
    if (st.empty()) throw Error("I can't get a type - stack is empty");
    Type res = st.top();
    st.pop();
    return res;
}

