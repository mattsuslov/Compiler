#include "LA.h"
#include "SA.h"

int main() {
    std::cout << "Token analysis...\n";
	LA la = LA();
	std::vector<Token> t = la.fact_to_tokens();
    std::cout << "OK" << std::endl;
//	la.print_tokens(std::cout, t);
    std::cout << "Syntax analysis...\n";
    SA sa = SA(t);
    std::cout << "OK" << std::endl;
	return 0;
}


