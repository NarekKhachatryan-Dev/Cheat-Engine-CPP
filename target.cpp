#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

int main() {
    int target = 12345;

    std::cout << "Target PID: "<< getpid() << std::endl;
    std::cout << "Target Address: " << &target << std::endl;
    std::cout << "Target value: " << target << std::endl;

    std::cout << "\nWaiting for 1 to continue programm: ";
    int end = 0;
    while(end != 1) {
        std::cin >> end;
    }

    std::cout << "Target: " << target << std::endl;

    return 0;
}