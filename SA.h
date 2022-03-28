#ifndef SA_H_
#define SA_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <stack>
#include "Token.h"

class FIRSTConstructor {
public:
    FIRSTConstructor(const std::string& filename, std::map<std::string, std::vector<std::string>>& f);
};

class Semantic {
public:
    struct Type;
    struct FSignature;
    int row = 0;

    struct Type {
        std::map<std::string, Type> fields;
        std::map<std::string, std::vector<FSignature>> ftid;
        std::string name;
        int ptr_num;
        bool is_ref;

        Type (std::string name0, int ptr_num0=0, bool is_ref0=false) {
            name = name0;
            ptr_num = ptr_num0;
            is_ref = is_ref0;
        }

        Type() = default;
        std::string GetName () {
            std::string res = name;
            for (int i = 0; i < ptr_num; ++i) {
                res += '*';
            }
            if (is_ref) {
                res += '&';
            }
            return res;
        }

        friend bool operator==(const Type& lhs, const Type& rhs) {
            return lhs.name == rhs.name && lhs.ptr_num == rhs.ptr_num; // сравнение s_ref?
        }

        bool check_method(const std::string& name) const;

        bool check_method(const std::string& name, const FSignature& sign);

        FSignature getSignature(const std::string& name, const FSignature& seg);

        Semantic::Type check_field(std::string name);

        bool isContainSignature(const std::vector<FSignature>& funcs, const FSignature& seg) const;

        void put_ftid(const std::pair<std::string, FSignature>& id);
    };

    struct FSignature {
        std::vector<Type> params;
        Type ret_type;
        friend bool operator==(const FSignature& a, const FSignature& b) {
            return a.params == b.params && a.ret_type == b.ret_type;
        }
        friend bool operator!=(const FSignature& a, const FSignature& b) {
            return !(a == b);
        }
    };

    struct TID {
        std::map<std::string, Type> data;
        TID* par = nullptr;
    };

    TID* getCurTid() const;

    std::map<std::string, Type> custom_type;

    bool isContainSignature(const std::vector<FSignature>& funcs, const FSignature& seg) const;

    FSignature getSignature(const std::vector<FSignature>& funcs, const FSignature& seg) const;

    FSignature getSignature(const std::string& name, const FSignature& seg);

    bool check_func(const std::string& name) const;

    bool check_func(const std::string& name, const FSignature& sign);

    bool check_struct(const std::string& name);

    void extend_tid();

    void put_ftid(const std::pair<std::string, FSignature>& id);

    void push_type(Type type);

    Type pop_type();

    void check_op(const std::string& op);

    void put_id(const std::string& id, Type type);

    void eq_type(Type type);

    Type check_id(const std::string& id) const;

    void del_tid();

    static Semantic& inst();

    Type type;
    std::string id;
private:
    Semantic() = default;

    std::map<std::string, std::vector<FSignature>> ftid;

    TID* cur_tid = new TID;

private:
    std::stack<Type> st;
};

class Poliz {
public:
    void push_operand(const std::string& str) {
        kim.push_back({str});
        print_self();
    }

    void put_operand(int i, const std::string& str) {
        kim[i] = {str};
        print_self();
    }

    void push_operation(const std::string& str, int prior, int cnt, bool is_right_ass) {
        if (str == ")") {
            while (cock.top().lex != "(") {
                Token x = cock.top();
                cock.pop();
                kim.push_back(x);
            }
            cock.pop();
        } else if (is_right_ass) {
            while (!cock.empty() && prior > cock.top().prior) {
                Token x = cock.top();
                cock.pop();
                kim.push_back(x);
            }
            cock.push({str, true, is_right_ass, cnt, prior});
        } else {
            while (!cock.empty() && prior >= cock.top().prior) {
                Token x = cock.top();
                cock.pop();
                kim.push_back(x);
            }
            cock.push({str, true, is_right_ass, cnt, prior});
        }
    }

    void finish_poliz() {
        while (!cock.empty()) {
            kim.push_back(cock.top());
            cock.pop();
        }
        print_self();
//        std::cout << calc() << std::endl;
    }

    int calc();

    int get_current_address() {
        return kim.size();
    }

    void print_self() {
        for (int i = 0; i < kim.size(); ++i) {
            std::cout << kim[i].lex << " ";
        }
        std::cout << std::endl;
    }

    std::string devalue(std::string str) {
        if (value.count(str)) {
            return value[str].second;
        }
        return str;
    }

private:
    struct Token {
        std::string lex;
        bool is_operation = false;
        bool is_right_ass = false;
        int cnt = 0;
        int prior = 0;
    };

    std::stack<Token> cock;
    std::vector<Token> kim;
    std::map<std::string, std::pair<Semantic::Type, std::string>> value;
};

class Generation {
public:
    Poliz poliz;

    void push_exp(const std::string& str) {
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
        } else if (str == "*") {
            poliz.push_operation("*", 4, 2, 0);
        } else if (str == "/") {
            poliz.push_operation("/", 4, 2, 0);
        } else if (str == "%") {
            poliz.push_operation("%", 4, 2, 0);
        } else if (str == "=") {
            poliz.push_operation("=", 12, 2, 1);
        } else if (str == "<") {
            poliz.push_operation("<", 6, 2, 0);
        } else {
            poliz.push_operand(str);
        }
        poliz.print_self();
    }

    void push_op(const std::string& str) {
        if (str == "jmp") {
            poliz.finish_poliz();
            poliz.push_operation("jmp", 20, 1, 0);
            poliz.finish_poliz();
        } else if (str == "jf") {
            poliz.finish_poliz();
            poliz.push_operation("jf", 20, 2, 0);
            poliz.finish_poliz();
        } else {

        }
        poliz.print_self();
    }

    static Generation& inst() {
        static Generation res = Generation();
        return res;
    }

private:
    Generation() = default;

};




class SA {
public:
	SA();
	void analize();

	void operator()() {
	    analize();
	}

private:
	Token cur;
	int ind = 0;
	int row = 1, col = 1;
    std::map<std::string, std::vector<std::string>> first_;
    bool first_equals(const std::string& str, const std::string& target);
    void GetToken();

    //Grammar
    void Program();

    void Struct();
    void StructBody(Semantic::Type& t);
    void Import();

    void Func();
    void Method(Semantic::Type& acc);
    void Type();
    void EType();
    void EEType();
    void EEEType();
    void Name();
    std::vector <Semantic::Type> Params();

    void Try();
    void CodeBlock();

    void Block();
    void Operator();
    void NonExtendedOperator();
    void For();
    void Definition();
    void If();
    void While();
    void Return();
    void Throw();

    void Prior1();
    void Prior1_dot();
    void Prior_dot();
    void Prior_dot_after();

    void Prior2();
    void Prior3();
    void Prior4();
    void Prior5();
    void Prior6();
    void Prior7();
    void Prior8();
    void Prior9();
    void Prior10();
    void Prior11();
    void Prior12();
    void Exp();

    std::vector<Semantic::Type> Enumeration();
};

#endif /* SA_H_ */
