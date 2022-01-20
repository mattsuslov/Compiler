#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>
#include <queue>
#include <thread>

enum {
	Res,
	Name,
	Float,
	Integer,
	Oper,
	Punct,
	String,
	Comment,
	Other
};

enum {
	Nothing,
	OneLetter,
	NotTerm
};

enum  {
    Wait,
    Go,
    End
};

class Token {
public:
	std::string data = "";
	int type = Other;

    static std::vector<Token> tokens;
    static int state;
    static std::mutex mutex;
};




#endif /* TOKEN_H_ */
