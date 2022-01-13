#include "LA.h"
#include "SA.h"

int main() {
	LA la = LA();
	std::vector<Token> t = la.fact_to_tokens();
	la.print_tokens(std::cout, t);
	SA sa = SA(t);
	return 0;
}


