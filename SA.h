#ifndef SA_H_
#define SA_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include "Token.h"

class FIRSTConstructor {
public:
    FIRSTConstructor(const std::string& filename, std::unordered_map<std::string, std::vector<std::string>>& f);
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
    std::unordered_map<std::string, std::vector<std::string>> first_;
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
    void Params();

    void Try();
    void CodeBlock();

    void Block();
    void Operator();
    void For();
    void Definition();
    void If();
    void While();
    void Return();

    void Prior1();
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

    void Enumeration();
};

#endif /* SA_H_ */
