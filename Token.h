#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>


enum {
	Res,
	Name,
	Float,
	Integer,
	Oper,
	Punct,
	Other
};

enum {
	Nothing,
	OneLetter,
	NotTerm
};

class Token {
public:
	std::string data = "";
	int type = Other;
};


#endif /* TOKEN_H_ */
