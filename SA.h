#ifndef SA_H_
#define SA_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include "Token.h"

class FIRSTConstructor {
public:
    FIRSTConstructor(const std::string& filename, std::map<std::string, std::vector<std::string>>& f);
};

class SA {
public:
	SA(const std::vector<Token>& t);

private:
	Token cur;
	int ind = 0;
	std::vector<Token> tokens;
    std::map<std::string, std::vector<std::string>> first_;
    bool first_equals(std::string first, std::string target);
    void GetToken();

    //Grammar
    void Program();

    void Struct();
    void Import();

    void Func();
    void Type();
    void Name();
    void Params();

    void Block();
    void Operator();
    void For();
    void Definition();
    void If();
    void While();

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