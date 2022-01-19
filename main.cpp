#include "LA.h"
#include "SA.h"
#include <thread>

double sum = 0;
std::vector<Token> Token::tokens = std::vector<Token>();
int Token::state = Wait;
std::mutex Token::mutex = std::mutex();

void test() {
    Token::tokens.clear();
    Token::state = Wait;

    auto st_time = std::chrono::high_resolution_clock::now();

    LA la = LA();
    SA sa = SA();

    la();
    sa();
//    std::thread la_th = std::thread( la );
//    std::thread sa_th = std::thread( sa );
//    la_th.join();
//    sa_th.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> tim = end_time - st_time;
    std::cout << tim.count() << std::endl;
    sum += tim.count();
    std::cout << "---------------" << std::endl;
}

int main() {
    freopen("test1_simple.txt", "w", stdout);
    int N = 100;
    std::cout << "N: " << N << std::endl;
    auto st_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        test();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> tim = end_time - st_time;
    std::cout << "Testing time: " << tim.count() << std::endl;
    sum /= N;
    std::cout << "Average: " << sum << std::endl;
	return 0;
}


