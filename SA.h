#ifndef SA_H_
#define SA_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <stack>
#include "Token.h"

static std::vector <std::string> basetype_names {"int", "bool", "void", "string",
                                                 "double", "char", "float", "point"};

class FIRSTConstructor {
public:
    FIRSTConstructor(const std::string& filename, std::map<std::string, std::vector<std::string>>& f);
};

class Semantic {
public:
    enum BASE_TYPE {
        INT,
        BOOL,
        VOID,
        STRING,
        DOUBLE,
        CHAR,
        FLOAT,
        POINTER
    };


    struct Type {
        Type(BASE_TYPE baseType) {
            type.push(baseType);
            type_name = basetype_names[baseType];
        }
        void Push(BASE_TYPE baseType) {
            type.push(baseType);
            if (baseType == POINTER) {
                type_name += "*";
            } else {
                type_name += basetype_names[baseType]; // а бывает ли вообще такое?
            }
        }
        Type() = default;
        std::string type_name;
        std::stack<BASE_TYPE> type;
        std::map<std::string, Type> fields;


        friend bool operator<(const Type& lhs, const Type& rhs) {
            return lhs.type < rhs.type;
        }
        friend bool operator==(const Type& lhs, const Type& rhs) {
            return lhs.type == rhs.type && lhs.fields == rhs.fields;
        }
        friend bool operator!=(const Type& lhs, const Type& rhs) {
            return !(lhs == rhs);
        }
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
    void StructBody();
    void Import();

    void Func();
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

    void Prior1();
    void Prior12_dot();
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
