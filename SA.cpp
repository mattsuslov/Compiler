#include "SA.h"
#include <regex>
#include <cmath>
#include <iostream>
#include "Token.h"
#include "Error.h"
#include "Logger.h"


SA::SA() {
    FIRSTConstructor("FIRST.txt", first_);
}

Semantic& sem = Semantic::inst();
Generation& gen = Generation::inst();

void SA::Definition() {
    EEEType();
    Name();
    sem.push_type(sem.type);
    std::string id = sem.id;
    Semantic::Type t = sem.type;

    gen.push_exp(id);
    if (cur.data == "=") {
        GetToken();
        Prior12();
        sem.check_op("=");

        gen.push_exp("=");
    } else {
        gen.push_exp("0");
        gen.push_exp("=");
    }

    sem.put_id(id, t);

    while (cur.data == ",") {
        GetToken();
        gen.push_exp(",");
        Name();

        id = sem.id;
        t = sem.type;
        gen.push_exp(id);
        if (cur.data == "=") {
            GetToken();
            Prior12();
            sem.check_op("=");

            gen.push_exp("=");
        } else {
            gen.push_exp("0");
            gen.push_exp("=");
        }

        sem.put_id(id, t);
    }
}

void SA::While() {
    sem.extend_tid();
    if (cur.data != "while") throw ExpectedSymbol(row, col, "while", cur.data.c_str());
    GetToken();

    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();

    int p1 = gen.poliz.get_current_address();
    Exp();
    sem.eq_type(Semantic::Type("bool"));
    gen.poliz.finish_poliz();
    int p2 = gen.poliz.get_current_address();
    gen.push_exp("nope");
    gen.push_op("jf");

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();


    NonExtendedOperator();
    sem.del_tid();

    gen.push_exp(std::to_string(p1));
    gen.push_op("jmp");
    gen.poliz.put_operand(p2, std::to_string(gen.poliz.get_current_address()));
}

std::vector<Semantic::Type> SA::Enumeration() {
    std::vector<Semantic::Type> res;

    Exp();
    res.push_back(sem.pop_type());

    while (cur.data == ",") {
        GetToken();
        gen.push_exp(",");
        Exp();
        res.push_back(sem.pop_type());
    }

    return res;
}

void SA::Struct() {
    sem.extend_tid();
    if (cur.data != "struct") throw ExpectedSymbol(row, col, "struct", cur.data.c_str());
    GetToken();

    Name();
    std::string struct_name = sem.id;
    if (sem.check_struct(struct_name)) throw Error(row, "Struct '" + struct_name + "' already exists");

    if (cur.data != "{") throw ExpectedSymbol(row, col, "{", cur.data.c_str());
    GetToken();

    if (cur.data == "}") {
        GetToken();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
        return;
    }

    Semantic::Type t(struct_name);
    StructBody(t);

    if (cur.data != "}") throw ExpectedSymbol(row, col, "}", cur.data.c_str());
    GetToken();
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();

    Semantic::TID* cur_tid = sem.getCurTid();
    for (auto el: cur_tid->data) {
        t.fields[el.first] = el.second;
    }
    sem.custom_type[struct_name] = t;
    sem.del_tid();
}

void SA::Type() {
    if (cur.data == "int" || cur.data == "bool" ||
        cur.data == "char" || cur.data == "double" || cur.data == "void" || cur.data == "float") {
        if (cur.data == "int") sem.type = Semantic::Type("int");
        if (cur.data == "bool") sem.type = Semantic::Type("bool");
        if (cur.data == "char") sem.type = Semantic::Type("char");
        if (cur.data == "double") sem.type = Semantic::Type("double");
        if (cur.data == "void") sem.type = Semantic::Type("void");
        if (cur.data == "float") sem.type = Semantic::Type("float");
        GetToken();
    } else if (first_equals("name", cur.data)) {
        Name();

        if (!sem.check_struct(sem.id)) {
            throw Error(row,"Undefined type '" + sem.id + "'");
        }
        sem.type = sem.custom_type[sem.id];
    } else {
        throw Error(row, "Declared types have to begin with letter or _ -> " + cur.data);
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
    sem.eq_type(Semantic::Type("bool"));
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
    gen.push_exp("(");
    GetToken();

    Exp();
    sem.eq_type(Semantic::Type("bool"));

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    gen.push_exp(")");

    int p1 = gen.poliz.get_current_address();
    gen.push_exp("nope");
    gen.push_op("jf");


    GetToken();

    Operator();
    int p2 = gen.poliz.get_current_address();
    gen.push_exp("nope");
    gen.push_op("jmp");

    if (cur.data != "else") {
        sem.del_tid();
        gen.poliz.put_operand(p1, std::to_string(gen.poliz.get_current_address()));
        gen.poliz.put_operand(p2, std::to_string(gen.poliz.get_current_address()));
        return;
    }
    GetToken();

    gen.poliz.put_operand(p1, std::to_string(gen.poliz.get_current_address()));
    Operator();
    sem.del_tid();
    gen.poliz.put_operand(p2, std::to_string(gen.poliz.get_current_address()));
    gen.poliz.finish_poliz();
}

void SA::Throw() {
    GetToken();
    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();
    Enumeration();
    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();
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
        gen.poliz.finish_poliz();
    } else if (first_equals("exp", cur.data)) {
        Enumeration();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
        gen.poliz.finish_poliz();
    } else if (first_equals("definition", cur.data)) {
        Definition();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
        gen.poliz.finish_poliz();
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
    } else if (first_equals("throw", cur.data)) {
        Throw();
    } else {
        throw Error(row, "Unknown operator -> " + cur.data);
    }
}

void SA::Name() {
    if (cur.type == 1) { // 1 - Name
        sem.id = cur.data;
        GetToken();
        return;
    } else {
        throw Error(row,"Expected name, found '" + cur.data + "'. Name can begin with letter or _" );
    }
}


std::vector <Semantic::Type> SA::Params() {
    std::vector <Semantic::Type> res;
    EEEType();
    Name();

    sem.put_id(sem.id, sem.type);
    std::string name = sem.id;

    res.push_back(sem.type);
    while (cur.data == ",") {
        GetToken();
        auto tmp = Params();
        for (auto el : tmp) {
            res.push_back(el);
        }
    }
    gen.push_exp(name);
    gen.push_exp("pop");
    gen.push_exp(",");
    return res;
}

void SA::Func() {
    EEEType();
    Name();

    std::string fname = sem.id;
    Semantic::Type ret_type = sem.type;
    sem.extend_tid();
    gen.got[fname] = gen.poliz.get_current_address();

    if (cur.data != "(") throw ExpectedSymbol(row, col, "(", cur.data.c_str());
    GetToken();
    gen.push_exp("(");

    Semantic::FSignature tmp;
    tmp.ret_type = ret_type;
    if (cur.data != ")") {
        tmp.params = Params();
    }
    sem.put_ftid({fname, tmp});

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();
    gen.push_exp(")");


    Block();
    gen.push_op("ret");

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
        sem.row = row;
        GetToken();
    }
    if (cur.type == Comment) {
        for (char ch: cur.data) {
            if (ch == '\n') {
                col = 1;
                row++;
                sem.row = row;
            } else {
                col++;
            }
        }
        GetToken();
    }
}

bool SA::first_equals(const std::string& str, const std::string& target) {
    if (str == "name") return cur.type == 1;
    if (str == "const") return (cur.type == Integer || cur.type == Float || cur.type == String || cur.type == Char);
    if (str == target) return true;
    for (int i = 0; i < first_[str].size(); ++i) {
        auto& el = first_[str][i];
        if (el == "name" && cur.type == 1) return true;
        if (el == "const" && (cur.type == Integer || cur.type == Float || cur.type == String || cur.type == Char)) return true;
        if (el == target) return true;
    }
    return false;
}




void SA::Prior1() {
    if (cur.data == "(") {
        gen.push_exp("(");
        GetToken();

        Prior12();

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        gen.push_exp(")");
        GetToken();
    } else {
        Name();

        std::string name = sem.id;

        if (cur.data == "(") {
            if (!sem.check_func(name)) throw Error(row, "Undefined function name '" + name + "' ");
            GetToken();

            Semantic::FSignature res;
            gen.push_exp("(");
            if (cur.data != ")") {
                res.params = Enumeration();
            }

            if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
            GetToken();
            gen.push_exp(")");
            if (!sem.check_func(name, res)) throw Error(row, "Call to undefined function '" + name + "' ");
            sem.push_type(sem.getSignature(name, res).ret_type);
            gen.push_exp(std::to_string(gen.got[name]));
            gen.push_exp("call");
        } else if (cur.data == "[") {
            Semantic::Type type = sem.check_id(name);
            if (type.name == "") throw Error(row,"Undefined array '" + sem.id + "' ");
            if (type.ptr_num == 0) throw Error(row,"Impossible indexing of '" + sem.id + "' ");

            while (cur.data == "[") {
                GetToken();

                Exp();
                sem.eq_type(Semantic::Type("int"));
                sem.pop_type();

                if(cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
                GetToken();
            }

            type.ptr_num--;
            sem.push_type(type);
        } else {
            Semantic::Type type = sem.check_id(name);
            if (type.name == "") throw Error(row,"Undefined var '" + sem.id + "' ");

            sem.push_type(type);
            gen.push_exp(name);
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
            Prior_dot();
        } else {
            if (cur.type == Integer) {
                sem.push_type(Semantic::Type("int"));
                gen.push_exp(cur.data);
            } else if (cur.type == Float) {
                sem.push_type(Semantic::Type("float"));
                gen.push_exp(cur.data);
            } else if (cur.type == String) {
                sem.push_type(Semantic::Type("char", 1));
                gen.push_exp(cur.data);
            } else if (cur.type == Char) {
                sem.push_type(Semantic::Type("char", 0));
                gen.push_exp(cur.data);
            } else if (cur.type == Res && (cur.data == "true" || cur.data == "false") ) {
                sem.push_type(Semantic::Type("bool"));
                gen.push_exp(cur.data);
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

        gen.push_exp(op);
        Prior3();
        sem.check_op(op);
    }
}

void SA::Prior4() {
    Prior3();
    if (cur.data == "*" || cur.data == "/" || cur.data == "%") {
        std::string op = cur.data;
        GetToken();
        gen.push_exp(op);
        Prior4();

        sem.check_op(op);
    }
}

void SA::Prior5() {
    Prior4();
    if (cur.data == "+" || cur.data == "-") {
        std::string op = cur.data;
        GetToken();
        gen.push_exp(op);
        Prior5();
        sem.check_op(op);
    }
}

void SA::Prior6() {
    Prior5();
    if (cur.data == ">" || cur.data == "<" || cur.data == ">=" || cur.data == "<=") {
        std::string op = cur.data;
        GetToken();
        gen.push_exp(op);
        Prior6();
        sem.check_op(op);
    }
}

void SA::Prior7() {
    Prior6();
    if (cur.data == "==" | cur.data == "!=") {
        std::string op = cur.data;
        GetToken();
        gen.push_exp(op);
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
        gen.push_exp("&&");
        Prior10();
        sem.check_op("&&");
    }
}

void SA::Prior11() {
    Prior10();
    if (cur.data == "||") {
        GetToken();
        gen.push_exp("||");
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
        gen.push_exp(op);
    }
}

void SA::Exp() {
    Prior12();
}

void SA::EType() {
    Type();
    Semantic::Type t = sem.type;
    bool is_array = false;
    while (cur.data == "[") {
        is_array = true;
        GetToken();

        Exp();
        sem.eq_type(Semantic::Type("int"));
        sem.pop_type();

        if (cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
        GetToken();
    }
    if (is_array) {
        t.ptr_num++;
    }
    sem.type = t;
}

void SA::EEType() {
    EType();
    while (cur.data == "*") {
        sem.type.ptr_num++;
        GetToken();
    }
}

void SA::EEEType() {
    EEType();
    if (cur.data == "&&" || cur.data == "&") {
        GetToken();
        sem.type.is_ref = true;
    }
}

void SA::Return() {
    if (cur.data != "return") throw ExpectedSymbol(row, col, "return", cur.data.c_str());
    GetToken();

    if (cur.data != ";") Exp();
    if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
    GetToken();

    gen.push_op("ret");
}

void SA::StructBody(Semantic::Type& t) {
    int ind_mem = ind;
    Token tok_mem = cur;
    EEEType();
    Name();
    ind = ind_mem;
    if (cur.data == "(") {
        cur = tok_mem;
        Method(t);
    } else  {
        cur = tok_mem;
        Definition();
        if (cur.data != ";") throw ExpectedSymbol(row, col, ";", cur.data.c_str());
        GetToken();
    }
    if (cur.data == "}") return;
    StructBody(t);
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
        Params();

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
   // Program();
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
        throw Error(row, "Unknown operator -> " + cur.data);
    }
}

void SA::Prior1_dot() {
    Name();
    std::string name = sem.id;

    Semantic::Type prev_type = sem.pop_type();


    if (cur.data == "(") {
        if (!prev_type.check_method(name)) throw Error(row,"Undefined method name '" + name + "' ");
        GetToken();

        Semantic::FSignature res;
        if (cur.data != ")") {
            res.params = Enumeration();
        }

        if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
        GetToken();

        if (!prev_type.check_method(name, res)) throw Error(row,"Call to undefined function '" + name +"' ");
        sem.push_type(prev_type.getSignature(name, res).ret_type);

    } else if (cur.data == "[") {
        Semantic::Type type = prev_type.check_field(name);
        if (type.name == "") throw Error(row,"Undefined array '" + sem.id + "' ");
        if (type.ptr_num == 0) throw Error(row,"Impossible indexing of '" + sem.id + "' ");

        while (cur.data == "[") {
            GetToken();

            Exp();
            sem.eq_type(Semantic::Type("int"));
            sem.pop_type();

            if(cur.data != "]") throw ExpectedSymbol(row, col, "]", cur.data.c_str());
            GetToken();
        }

        type.ptr_num--;
        sem.push_type(type);
    } else {
        Semantic::Type type = prev_type.check_field(name);
        if (type.name == "") throw Error(row,"Undefined var '" + sem.id + "' ");

        sem.push_type(type);
    }
}

void SA::Prior_dot() {
    Prior1();

    if (cur.data == ".") {
        GetToken();
        Prior_dot_after();
    }
}

void SA::Prior_dot_after() {
    Prior1_dot();

    if (cur.data == ".") {
        GetToken();
        Prior_dot_after();
    }
}

void SA::Method(Semantic::Type& acc) {
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
    acc.put_ftid({fname, tmp});

    if (cur.data != ")") throw ExpectedSymbol(row, col, ")", cur.data.c_str());
    GetToken();

    Block();

    sem.del_tid();
}


FIRSTConstructor::FIRSTConstructor(const std::string &filename, std::map<std::string, std::vector<std::string>> &f) {
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
    if (op == "+" || op == "-") {
        if (st.size() < 2) throw Error(row,"'" + op + "' is a binary operation");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1 == Semantic::Type("int") && t2 == Semantic::Type("int")) {
            st.push(t1);
        } else if (t1.ptr_num > 0 && t2 == Semantic::Type("int")) {
            st.push(t1);
        } else if (t1 == Semantic::Type("int") && t2.ptr_num > 0) {
            st.push(t2);
        } else {
            throw Error(row,"Operands must be integers");
        }
    } else if (op == "*" || op == "/" || op == "^") {
        if (st.size() < 2) throw Error(row,"'" + op + "' is a binary operation");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1 == Semantic::Type("int")  && t2 == Semantic::Type("int")) {
            st.push(t1);
        } else {
            throw Error(row, "Operands must be integers");
        }
    } else if (op == "+un" || op == "-un" || op == "++un" || op == "--un") {
        if (st.size() < 1) throw Error(row, "'" + op + "' expected at least one operand");
        Type t1 = st.top(); st.pop();
        if (t1 == Semantic::Type("int") ) {
            st.push(Semantic::Type("int") );
        } else {
            throw Error(row,"Operands must be integers");
        }
    } else if (op == "*un") {
        if (st.size() < 1) throw Error(row,"'" + op + "' expected at least one operand");
        Type t1 = st.top(); st.pop();
        if (t1.ptr_num > 0) {
            t1.ptr_num--;
            st.push(t1);
        } else {
            throw Error(row,"Operands must be integers");
        }
    } else if (op == "&un") {
        if (st.size() < 1) throw Error(row, "'" + op + "' expected at least one operand!");
        Type t1 = st.top(); st.pop();
        t1.ptr_num++;
        st.push(t1);
    } else if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        if (st.size() < 2) throw Error(row,"'" + op + "' is a binary operation!");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1 == Semantic::Type("bool") && t2 == Semantic::Type("bool")) {
            st.push(Semantic::Type("bool"));
        } else if (t1 == Semantic::Type("int") && t2 == Semantic::Type("int")) {
            st.push(Semantic::Type("bool"));
        } else {
            throw Error(row,"Operands must be bool");
        }
    } else if (op == "=" || op == "+=" || op == "-="
               || op == "*=" || op == "/=" ||  op == "%=" || op == "^="
               || op == "&=" || op == "|=") {
        if (st.size() < 2) throw Error(row, "'" +op + "' is a binary operation");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1 == t2) {
            st.push(t1);
        } else {
            throw Error(row,"You can't put " + t1.GetName() + " into " + t2.GetName());
        }
    } else if (op == "|" || op == "&") {
        if (st.size() < 2) throw Error(row,op + " is a binary operation");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1 == Semantic::Type("int")  && t2 == Semantic::Type("int") ) {
            st.push(Semantic::Type("int"));
        } else {
            throw Error(row,"Operands must be integers");
        }
    } else if (op == "||" || op == "&&") {
        if (st.size() < 2) throw Error(row, "'" +op + "' is a binary operation");
        Type t1 = st.top(); st.pop();
        Type t2 = st.top(); st.pop();
        if (t1 == Semantic::Type("bool")  && t2 == Semantic::Type("bool") ) {
            st.push(Semantic::Type("bool"));
        } else {
            throw Error(row,"Operands must be bool");
        }
    } else {
        throw Error(row,"Unknown operation '" + op + "'");
    }
}

void Semantic::put_id(const std::string &id, Semantic::Type type) {
    if (cur_tid->data.count(id)) throw Error(row,id + " has been already defined");
    cur_tid->data[id] = type;
}



void Semantic::eq_type(Semantic::Type type) {
    if (st.empty()) return;

    if (type.GetName() != st.top().GetName()) {
        throw Error(row,"Expected " + type.GetName() + ",found " + st.top().GetName());
    }
}

void Semantic::put_ftid(const std::pair<std::string, FSignature>& id) {
    if (ftid.count(id.first) && isContainSignature(ftid[id.first], id.second))
        throw Error(row,"Ambiguous definition of '" + id.first + "'");

    ftid[id.first].push_back(id.second);
}

Semantic::Type Semantic::pop_type() {
    if (st.empty()) throw Error("I can't get a type - stack is empty");
    Type res = st.top();
    st.pop();
    return res;
}

bool Semantic::isContainSignature(const std::vector<FSignature> &funcs, const Semantic::FSignature &seg) const {
    for (const auto &elem: funcs) {
        if (elem.params == seg.params) return true;
    }
    return false;
}

Semantic::FSignature Semantic::getSignature(const std::vector<FSignature> &funcs, const Semantic::FSignature &seg) const {
    for (const auto &elem: funcs) {
        if (elem.params == seg.params) return elem;
    }
    return seg;
}

Semantic::FSignature Semantic::getSignature(const std::string &name, const Semantic::FSignature &seg) {
    const auto& funcs = ftid[name];
    for (const auto &elem: funcs) {
        if (elem.params == seg.params) return elem;
    }
    return seg;
}

bool Semantic::check_func(const std::string &name) const {
    return ftid.count(name);
}

bool Semantic::check_func(const std::string &name, const Semantic::FSignature &sign) {
    return check_func(name) && isContainSignature(ftid[name], sign);
}

void Semantic::extend_tid() {
    TID* q = new TID;
    q->par = cur_tid;
    cur_tid = q;
}

void Semantic::push_type(Semantic::Type type) {
    st.push(type);
}

Semantic::Type Semantic::check_id(const std::string &id) const {
    TID* ptr = cur_tid;
    while (ptr) {
        if (ptr->data.count(id)) return ptr->data[id];
        ptr = ptr->par;
    }
    return Type();
}

void Semantic::del_tid() {
    if (!cur_tid) return;
    TID* tmp = cur_tid->par;
    delete cur_tid;
    cur_tid = tmp;
}

Semantic &Semantic::inst() {
    static Semantic res = Semantic();
    return res;
}

Semantic::TID *Semantic::getCurTid() const {
    return cur_tid;
}

bool Semantic::check_struct(const std::string &name) {
    return custom_type.count(name);
}

Semantic::Type Semantic::Type::check_field(std::string name) {
    if (!fields.count(name)) throw Error(sem.row, "Unknown field '" + name + "' ");
    return fields[name];
}

void Semantic::Type::put_ftid(const std::pair<std::string, FSignature> &id) {
    if (ftid.count(id.first) && isContainSignature(ftid[id.first], id.second))
        throw Error(sem.row, "Ambiguous definition of '" + id.first + "'");

    ftid[id.first].push_back(id.second);
}

bool Semantic::Type::isContainSignature(const std::vector<FSignature> &funcs, const Semantic::FSignature &seg) const {
    for (const auto &elem: funcs) {
        if (elem.params == seg.params) return true;
    }
    return false;
}

bool Semantic::Type::check_method(const std::string &name) const {
    return ftid.count(name);
}

bool Semantic::Type::check_method(const std::string &name, const Semantic::FSignature &sign) {
    return check_method(name) && isContainSignature(ftid[name], sign);
}

Semantic::FSignature Semantic::Type::getSignature(const std::string &name, const Semantic::FSignature &seg) {
    const auto& funcs = ftid[name];
    for (const auto &elem: funcs) {
        if (elem.params == seg.params) return elem;
    }
    return seg;
}

int Poliz::calc() {
    if (!gen.got.count("main")) {
        throw Error("I can't find start point. You need to create main function");
    }
    std::stack<Token> res;
    for (int i = gen.got["main"]; i < kim.size(); ++i) {
        Token x = kim[i];
        if (!x.is_operation) {
            res.push({x.lex});
        } else {
            std::vector<Token> operands;
            while (x.cnt--) {
                operands.push_back(res.top());
                res.pop();
            }

            if (x.lex == "+") {
                int ans = 0;
                for (Token el: operands) {
                    ans += std::stoi(devalue(el.lex));
                }
                res.push({std::to_string(ans)});
            } else if (x.lex == "-") {
                int ans = std::stoi(devalue(operands[1].lex)) - std::stoi(devalue(operands[0].lex));
                res.push({std::to_string(ans)});
            } else if (x.lex == "*") {
                int ans = std::stoi(devalue(operands[1].lex)) * std::stoi(devalue(operands[0].lex));
                res.push({std::to_string(ans)});
            } else if (x.lex == "/") {
                int ans = std::stoi(devalue(operands[1].lex)) / std::stoi(devalue(operands[0].lex));
                res.push({std::to_string(ans)});
            } else if (x.lex == "%") {
                int ans = std::stoi(devalue(operands[1].lex)) % std::stoi(devalue(operands[0].lex));
                res.push({std::to_string(ans)});
            } else if (x.lex == "^") {
                int ans = pow(std::stoi(devalue(operands[1].lex)), std::stoi(devalue(operands[0].lex)));
                res.push({std::to_string(ans)});
            }  else if (x.lex == "jmp") {
                int addr = std::stoi(devalue(operands[0].lex));
                i = addr - 1;
            } else if (x.lex == "jf") {
                int addr = std::stoi(devalue(operands[0].lex));
                if (operands[1].lex == "false") {
                    i = addr - 1;
                }
            } else if (x.lex == "=") {
                value[operands[1].lex] = {sem.check_id(devalue(operands[1].lex)), devalue(operands[0].lex)};
                res.push({devalue(operands[1].lex)});
            } else if (x.lex == "<") {
                if (std::stoi(devalue(operands[1].lex)) < std::stoi(devalue(operands[0].lex))) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == "<=") {
                if (std::stoi(devalue(operands[1].lex)) <= std::stoi(devalue(operands[0].lex))) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == ">") {
                if (std::stoi(devalue(operands[1].lex)) > std::stoi(devalue(operands[0].lex))) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == ">=") {
                if (std::stoi(devalue(operands[1].lex)) >= std::stoi(devalue(operands[0].lex))) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            }  else if (x.lex == "==") {
                if (std::stoi(devalue(operands[1].lex)) == std::stoi(devalue(operands[0].lex))) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == "!=") {
                if (std::stoi(devalue(operands[1].lex)) != std::stoi(devalue(operands[0].lex))) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == "&&") {
                bool lhs = (devalue(operands[1].lex) == "true");
                bool rhs = (devalue(operands[0].lex) == "true");
                if (lhs && rhs) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == "||") {
                bool lhs = (devalue(operands[1].lex) == "true");
                bool rhs = (devalue(operands[0].lex) == "true");
                if (lhs || rhs) {
                    res.push({"true"});
                } else {
                    res.push({"false"});
                }
            } else if (x.lex == "ret") {
                if (gen.ret_addr.empty()) return std::stoi(res.top().lex);
                i = gen.ret_addr.top() - 1;
                gen.ret_addr.pop();

                std::string ret_val = (devalue(res.top().lex));
                res.pop();
                res.push({ret_val});
            } else if (x.lex == "call") {
                int addr = std::stoi(operands[0].lex);
                gen.ret_addr.push(i + 1);
                i = addr - 1;
            } else if (x.lex == "pop") {
                value[operands[0].lex] = {sem.check_id(operands[0].lex), res.top().lex};
                res.pop();
            } else {
                std::cout << "Unknown operation" << std::endl;
            }
        }
    }

    return std::stoi(res.top().lex);
}

void Generation::push_exp(const std::string &str) {
    if (str == "+") {
        poliz.push_operation("+", 5, 2, 0);
    } else if (str == "-") {
        poliz.push_operation("-", 5, 2, 0);
    } else if (str == ";") {
        poliz.finish_poliz();
    } else if (str == "(") {
        poliz.push_operation("(", 100, 0, 0);
    } else if (str == ")") {
        poliz.push_operation(")", 100, 0, 0);
    } else if (str == ",") {
        poliz.push_operation(",", 100, 0, 0);
    } else if (str == "*") {
        poliz.push_operation("*", 4, 2, 0);
    } else if (str == "/") {
        poliz.push_operation("/", 4, 2, 0);
    } else if (str == "%") {
        poliz.push_operation("%", 4, 2, 0);
    } else if (str == "^") {
        poliz.push_operation("^", 3, 2, 1);
    } else if (str == "=") {
        poliz.push_operation("=", 12, 2, 1);
    } else if (str == "<") {
        poliz.push_operation("<", 6, 2, 0);
    } else if (str == "<=") {
        poliz.push_operation("<=", 6, 2, 0);
    } else if (str == ">") {
        poliz.push_operation(">", 6, 2, 0);
    } else if (str == ">=") {
        poliz.push_operation(">=", 6, 2, 0);
    }  else if (str == "==") {
        poliz.push_operation("==", 7, 2, 0);
    } else if (str == "!=") {
        poliz.push_operation("!=", 7, 2, 0);
    } else if (str == "&&") {
        poliz.push_operation("&&", 10, 2, 0);
    } else if (str == "||") {
        poliz.push_operation("||", 11, 2, 0);
    } else if (str == "call") {
        poliz.push_operation("call", 1, 1, 0);
    } else if (str == "pop") {
        poliz.push_operation("pop", 1, 1, 0);
    } else {
        poliz.push_operand(str);
    }
    poliz.print_self();
}

void Generation::push_op(const std::string &str) {
    if (str == "jmp") {
        poliz.finish_poliz();
        poliz.push_operation("jmp", 20, 1, 0);
        poliz.finish_poliz();
    } else if (str == "jf") {
        poliz.finish_poliz();
        poliz.push_operation("jf", 20, 2, 0);
        poliz.finish_poliz();
    } else if (str == "ret") {
        poliz.finish_poliz();
        poliz.push_operation("ret", 20, 0, 0);
        poliz.finish_poliz();
    } else {
        poliz.push_operand(str);
    }
    poliz.print_self();
}
